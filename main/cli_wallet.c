#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "argtable3/argtable3.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "esp32/rom/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http_parser.h"
#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"

#include "cli_wallet.h"

#include "client/api/v1/get_node_info.h"
#include "client/api/v1/send_message.h"
#include "wallet/wallet.h"

#define APP_WALLET_SEED CONFIG_WALLET_SEED
#define APP_NODE_URL CONFIG_IOTA_NODE_URL
#define APP_NODE_PORT CONFIG_IOTA_NODE_PORT

static const char *TAG = "wallet";

iota_wallet_t *wallet = NULL;

#if 0
#define DIM(x) (sizeof(x) / sizeof(*(x)))
static const char *iota_unit[] = {"Pi", "Ti", "Gi", "Mi", "Ki", "i"};
static const uint64_t peta_i = 1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL;

void print_iota(uint64_t value) {
  char value_str[32] = {};
  uint64_t multiplier = peta_i;
  int i = 0;
  for (i = 0; i < DIM(iota_unit); i++, multiplier /= 1000) {
    if (value < multiplier) {
      continue;
    }
    if (value % multiplier == 0) {
      sprintf(value_str, "%" PRIu64 "%s", value / multiplier, iota_unit[i]);
    } else {
      sprintf(value_str, "%.2f%s", (float)value / multiplier, iota_unit[i]);
    }
    break;
  }
  printf("%s", value_str);
}
#endif

// 0 on success
static int config_validation() {
  // URL parsing
  struct http_parser_url u;
  http_parser_url_init(&u);
  int parse_ret = http_parser_parse_url(APP_NODE_URL, strlen(APP_NODE_URL), 0, &u);
  if (parse_ret != 0) {
    ESP_LOGE(TAG, "validating endpoint error\n");
    return -1;
  }

  // seed length
  if (strcmp(CONFIG_WALLET_SEED, "random") != 0) {
    if (strlen(CONFIG_WALLET_SEED) != 64) {
      ESP_LOGE(TAG, "seed length is %d, should be 64\n", strlen(CONFIG_WALLET_SEED));
      return -1;
    }
  }
  return 0;
}

/* 'version' command */
static int fn_get_version(int argc, char **argv) {
  esp_chip_info_t info;
  esp_chip_info(&info);
  printf("IDF Version:%s\r\n", esp_get_idf_version());
  printf("Chip info:\r\n");
  printf("\tmodel:%s\r\n", CONFIG_IDF_TARGET);
  printf("\tcores:%d\r\n", info.cores);
  printf("\tfeature:%s%s%s%s%d%s\r\n", info.features & CHIP_FEATURE_WIFI_BGN ? ", 802.11bgn" : "",
         info.features & CHIP_FEATURE_BLE ? ", BLE" : "", info.features & CHIP_FEATURE_BT ? ", BT" : "",
         info.features & CHIP_FEATURE_EMB_FLASH ? ", Embedded-Flash:" : ", External-Flash:",
         spi_flash_get_chip_size() / (1024 * 1024), " MB");
  printf("\trevision number:%d\r\n", info.revision);
  printf("Wallet version: v%s\n", APP_WALLET_VERSION);
  return 0;
}

static void register_version() {
  const esp_console_cmd_t cmd = {
      .command = "version",
      .help = "Show the esp32 and wallet versions",
      .hint = NULL,
      .func = &fn_get_version,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'restart' command */
static int fn_restart(int argc, char **argv) {
  ESP_LOGI(TAG, "Restarting");
  esp_restart();
}

static void register_restart() {
  const esp_console_cmd_t cmd = {
      .command = "restart",
      .help = "System reboot",
      .hint = NULL,
      .func = &fn_restart,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'free' command */
static int fn_free_mem(int argc, char **argv) {
  printf("%d\n", esp_get_free_heap_size());
  return 0;
}

static void register_free() {
  const esp_console_cmd_t cmd = {
      .command = "free",
      .help = "Get the current size of free heap memory",
      .hint = NULL,
      .func = &fn_free_mem,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/* 'heap' command */
static int fn_heap_size(int argc, char **argv) {
  printf("heap info (SPI RAM): \n");
  heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
  printf("\nheap info (DEFAULT): \n");
  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
  return 0;
}

static void register_heap() {
  const esp_console_cmd_t heap_cmd = {
      .command = "heap",
      .help = "Get heap memory info",
      .hint = NULL,
      .func = &fn_heap_size,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&heap_cmd));
}

/* 'stack' command */
static int fn_stack_info(int argc, char **argv) {
  printf("%u tasks are running on the system\n", uxTaskGetNumberOfTasks());
  printf("Main stack size: %d, remaining %d bytes\n", CONFIG_MAIN_TASK_STACK_SIZE, uxTaskGetStackHighWaterMark(NULL));
  return 0;
}

static void register_stack_info() {
  const esp_console_cmd_t stack_info_cmd = {
      .command = "stack",
      .help = "Get system stack info",
      .hint = NULL,
      .func = &fn_stack_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&stack_info_cmd));
}

/* 'address' command */
static struct {
  struct arg_dbl *idx_start;
  struct arg_dbl *idx_end;
  struct arg_end *end;
} get_addr_args;

static int fn_get_address(int argc, char **argv) {
  byte_t addr_wit_version[IOTA_ADDRESS_BYTES] = {};
  char tmp_bech32_addr[100] = {};
  int nerrors = arg_parse(argc, argv, (void **)&get_addr_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_addr_args.end, argv[0]);
    return -1;
  }
  uint32_t start = (uint32_t)get_addr_args.idx_start->dval[0];
  uint32_t end = (uint32_t)get_addr_args.idx_end->dval[0];
  if (end < start) {
    ESP_LOGI(TAG, "invalid address range\n");
    return -1;
  }

  while (end >= start) {
    addr_wit_version[0] = ADDRESS_VER_ED25519;
    wallet_address_by_index(wallet, start, addr_wit_version + 1);
    address_2_bech32(addr_wit_version, wallet->bech32HRP, tmp_bech32_addr);
    printf("Addr[%" PRIu32 "]\n", start);
    // print ed25519 address without version filed.
    printf("\t");
    dump_hex_str(addr_wit_version + 1, ED25519_ADDRESS_BYTES);
    // print out
    printf("\t%s\n", tmp_bech32_addr);
    start++;
  }
  return 0;
}

static void register_get_address() {
  get_addr_args.idx_start = arg_dbl1(NULL, NULL, "<start>", "start index");
  get_addr_args.idx_end = arg_dbl1(NULL, NULL, "<end>", "end index");
  get_addr_args.end = arg_end(2);
  const esp_console_cmd_t get_address_cmd = {
      .command = "address",
      .help = "Get the address from an index range",
      .hint = " <start> <end>",
      .func = &fn_get_address,
      .argtable = &get_addr_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_address_cmd));
}

/* 'balance' command */
static struct {
  struct arg_dbl *idx_start;
  struct arg_dbl *idx_end;
  struct arg_end *end;
} get_balance_args;

static int fn_get_balance(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_balance_args);
  uint64_t balance = 0;
  uint32_t addr_start = 0;
  uint32_t addr_end = 0;
  if (nerrors != 0) {
    arg_print_errors(stderr, get_balance_args.end, argv[0]);
    return -1;
  }

  addr_start = get_balance_args.idx_start->dval[0];
  addr_end = get_balance_args.idx_end->dval[0];

  if (addr_end < addr_start) {
    ESP_LOGI(TAG, "invalid address range\n");
    return -1;
  }

  while (addr_end >= addr_start) {
    if (wallet_balance_by_index(wallet, addr_start, &balance) != 0) {
      ESP_LOGI(TAG, "get balance failed on %zu\n", addr_start);
      return -1;
    }
    printf("balance on address [%" PRIu32 "]: %" PRIu64 "i\n", addr_start, balance);
    addr_start++;
  }

  return 0;
}

static void register_get_balance() {
  get_balance_args.idx_start = arg_dbl1(NULL, NULL, "<start>", "start index");
  get_balance_args.idx_end = arg_dbl1(NULL, NULL, "<end>", "end index");
  get_balance_args.end = arg_end(2);
  const esp_console_cmd_t get_balance_cmd = {
      .command = "balance",
      .help = "Get the balance from a range of address index",
      .hint = " <start> <end>",
      .func = &fn_get_balance,
      .argtable = &get_balance_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_balance_cmd));
}

/* 'send' command */
static struct {
  struct arg_dbl *sender;
  struct arg_str *receiver;
  struct arg_dbl *balance;
  struct arg_end *end;
} send_msg_args;

static int fn_send_msg(int argc, char **argv) {
  int err = 0;
  char msg_id[IOTA_MESSAGE_ID_HEX_BYTES + 1] = {};
  char data[] = "sent from esp32 via iota.c";
  int nerrors = arg_parse(argc, argv, (void **)&send_msg_args);
  byte_t recv[IOTA_ADDRESS_BYTES] = {};
  if (nerrors != 0) {
    arg_print_errors(stderr, send_msg_args.end, argv[0]);
    return -1;
  }

  char *recv_addr = send_msg_args.receiver->sval[0];
  // validating receiver address
  if (strncmp("atoi", recv_addr, 4) == 0 || strncmp("iota", recv_addr, 4) == 0) {
    // convert bech32 address to binary
    if ((err = address_from_bech32(wallet->bech32HRP, recv_addr, recv))) {
      ESP_LOGE(TAG, "invalid bech32 address\n");
      return -1;
    }
  } else if (strlen(recv_addr) == IOTA_ADDRESS_HEX_BYTES) {
    // convert ed25519 string to binary
    if (hex2bin(recv_addr, strlen(recv_addr), recv + 1, ED25519_ADDRESS_BYTES) != 0) {
      ESP_LOGE(TAG, "invalid ed25519 address\n");
      return -1;
    }

  } else {
    ESP_LOGE(TAG, "invalid receiver address\n");
    return -1;
  }

  // balance = number * Mi
  uint64_t balance = (uint64_t)send_msg_args.balance->dval[0] * 1000000;

  if (balance > 0) {
    printf("send %" PRIu64 "Mi to %s\n", (uint64_t)send_msg_args.balance->dval[0], recv_addr);
  } else {
    printf("send indexation payload to tangle\n");
  }

  err = wallet_send(wallet, (uint32_t)send_msg_args.sender->dval[0], recv + 1, balance, "ESP32 Wallet", (byte_t *)data,
                    sizeof(data), msg_id, sizeof(msg_id));
  if (err) {
    printf("send message failed\n");
    return -1;
  }
  printf("Message Hash: %s\n", msg_id);
  return 0;
}

static void register_send_tokens() {
  send_msg_args.sender = arg_dbl1(NULL, NULL, "<index>", "Address index");
  send_msg_args.receiver = arg_str1(NULL, NULL, "<receiver>", "Receiver address");
  send_msg_args.balance = arg_dbl1(NULL, NULL, "<balance>", "balance");
  send_msg_args.end = arg_end(20);
  const esp_console_cmd_t send_msg_cmd = {
      .command = "send",
      .help = "send message to tangle",
      .hint = " <addr_index> <receiver> <balance>",
      .func = &fn_send_msg,
      .argtable = &send_msg_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&send_msg_cmd));
}

/* 'node_info' command */
static int fn_node_info(int argc, char **argv) {
  int err = 0;
  res_node_info_t *info = res_node_info_new();
  if (!info) {
    ESP_LOGE(TAG, "Create node info object failed\n");
    return -1;
  }

  if ((err = get_node_info(&wallet->endpoint, info)) == 0) {
    if (info->is_error) {
      printf("Error: %s\n", info->u.error->msg);
    } else {
      printf("Name: %s\n", info->u.output_node_info->name);
      printf("Version: %s\n", info->u.output_node_info->version);
      printf("isHealthy: %s\n", info->u.output_node_info->is_healthy ? "true" : "false");
      printf("Network ID: %s\n", info->u.output_node_info->network_id);
      printf("bech32HRP: %s\n", info->u.output_node_info->bech32hrp);
      printf("minPoWScore: %" PRIu64 "\n", info->u.output_node_info->min_pow_score);
      printf("Latest Milestone Index: %" PRIu64 "\n", info->u.output_node_info->latest_milestone_index);
      printf("Latest Milestone Timestamp: %" PRIu64 "\n", info->u.output_node_info->latest_milestone_timestamp);
      printf("Confirmed Milestone Index: %" PRIu64 "\n", info->u.output_node_info->confirmed_milestone_index);
      printf("Pruning Index: %" PRIu64 "\n", info->u.output_node_info->pruning_milestone_index);
      printf("MSP: %0.2f\n", info->u.output_node_info->msg_pre_sec);
      printf("Referenced MPS: %0.2f\n", info->u.output_node_info->referenced_msg_pre_sec);
      printf("Reference Rate: %0.2f%%\n", info->u.output_node_info->referenced_rate);
    }
  }
  res_node_info_free(info);
  return err;
}

static void register_node_info() {
  const esp_console_cmd_t node_info_cmd = {
      .command = "node_info",
      .help = "Show node info",
      .hint = NULL,
      .func = &fn_node_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&node_info_cmd));
}

//============= Public functions====================

void register_wallet_commands() {
  // system info
  register_free();
  register_heap();
  register_stack_info();
  register_version();
  register_restart();

  // wallet APIs
  register_node_info();
  register_get_balance();
  register_send_tokens();
  register_get_address();
}

int init_wallet() {
  byte_t seed[IOTA_SEED_BYTES] = {};

  if (config_validation() != 0) {
    return -1;
  }

  if (strcmp(CONFIG_WALLET_SEED, "random") == 0) {
    random_seed(seed);
  } else {
    size_t seed_len = strlen(CONFIG_WALLET_SEED);
    if (seed_len != IOTA_SEED_HEX_BYTES) {
      ESP_LOGI(TAG, "invalid seed length: %zu, expect 64 characters", seed_len);
      return -1;
    }
    hex2bin(CONFIG_WALLET_SEED, strlen(CONFIG_WALLET_SEED), seed, sizeof(seed));
  }

  // create wallet instance
  wallet = wallet_create(seed, "m/44'/4218'/0'/0'");
  if (!wallet) {
    ESP_LOGE(TAG, "wallet create failed\n");
    return -1;
  }

  // config wallet
  if (wallet_set_endpoint(wallet, APP_NODE_URL, APP_NODE_PORT) == 0) {
    if (wallet_update_bech32HRP(wallet) != 0) {
      ESP_LOGE(TAG, "update bech32HRP failed");
      wallet_destroy(wallet);
      return -1;
    }

    ESP_LOGI(TAG, "Wallet endpoint: %s", wallet->endpoint.url);
    ESP_LOGI(TAG, "Bech32HRP: %s", wallet->bech32HRP);
    return 0;
  }

  ESP_LOGE(TAG, "config endpoint failed");
  wallet_destroy(wallet);
  return -1;
}
