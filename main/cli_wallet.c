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

#include "client/api/v1/send_message.h"
#include "wallet/wallet.h"

#define APP_WALLET_SEED CONFIG_WALLET_SEED
#define APP_NODE_URL CONFIG_IOTA_NODE_URL
#define APP_NODE_PORT CONFIG_IOTA_NODE_PORT

static const char *TAG = "wallet";

iota_wallet_t *wallet = NULL;

#define DIM(x) (sizeof(x) / sizeof(*(x)))
static const char *iota_unit[] = {"Pi", "Ti", "Gi", "Mi", "Ki", "i"};
static const uint64_t peta_i = 1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL;

void print_iota(uint64_t value) {
  char value_str[32] = {};
  uint64_t multiplier = peta_i;
  int i;
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
      .help = "Get version of chip and wallet",
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
      .help = "Software reset of the chip",
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
  struct arg_dbl *index;
  struct arg_end *end;
} get_addr_args;

static int fn_get_address(int argc, char **argv) {
  byte_t addr_wit_version[IOTA_ADDRESS_BYTES] = {};
  char tmp_bech32_addr[100] = {};
  uint32_t addr_index = 0;
  int nerrors = arg_parse(argc, argv, (void **)&get_addr_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_addr_args.end, argv[0]);
    return -1;
  }
  addr_index = (uint32_t)get_addr_args.index->dval[0];
  addr_wit_version[0] = ADDRESS_VER_ED25519;
  wallet_address_by_index(wallet, addr_index, addr_wit_version + 1);
  address_2_bech32(addr_wit_version, "atoi", tmp_bech32_addr);
  printf("Addr[%" PRIu32 "]\n", addr_index);
  // print ed25519 address without version filed.
  printf("\t");
  dump_hex_str(addr_wit_version + 1, ED25519_ADDRESS_BYTES);
  // print out
  printf("\t%s\n", tmp_bech32_addr);
  return 0;
}

static void register_get_address() {
  // get_addr_args.address = arg_str1(NULL, NULL, "<ADDRESS>", "An Address hash");
  get_addr_args.index = arg_dbl1(NULL, NULL, "<index>", "Address index");
  get_addr_args.end = arg_end(1);
  const esp_console_cmd_t get_address_cmd = {
      .command = "address",
      .help = "Get the address from an index",
      .hint = " <index>",
      .func = &fn_get_address,
      .argtable = &get_addr_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_address_cmd));
}

/* 'balance' command */
static struct {
  struct arg_dbl *index;
  struct arg_end *end;
} get_balance_args;

static int fn_get_balance(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_balance_args);
  uint64_t balance = 0;
  uint32_t addr_index = 0;
  if (nerrors != 0) {
    arg_print_errors(stderr, get_balance_args.end, argv[0]);
    return -1;
  }

  addr_index = get_balance_args.index->dval[0];
  if (wallet_balance_by_index(wallet, addr_index, &balance) != 0) {
    return -1;
  }
  // printf("balance on address [%" PRIu32 "]: %" PRIu64 "\n", addr_index, balance);
  printf("balance on address [%" PRIu32 "]: ", addr_index);
  print_iota(balance);
  printf("\n");
  return 0;
}

static void register_get_balance() {
  // get_balance_args.address = arg_str1(NULL, NULL, "<ADDRESS>", "An Address hash");
  get_balance_args.index = arg_dbl1(NULL, NULL, "<index>", "Address index");
  get_balance_args.end = arg_end(1);
  const esp_console_cmd_t get_balance_cmd = {
      .command = "balance",
      .help = "Get the balance from an address index",
      .hint = " <index>",
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
  uint64_t balance = 0;
  char data[] = "sent from esp32 via iota.c";
  int nerrors = arg_parse(argc, argv, (void **)&send_msg_args);
  byte_t recv[IOTA_ADDRESS_BYTES] = {};
  if (nerrors != 0) {
    arg_print_errors(stderr, send_msg_args.end, argv[0]);
    return -1;
  }

  // convert receiver to binary
  if ((err = address_from_bech32("atoi", send_msg_args.receiver->sval[0], recv))) {
    printf("convert receiver address failed\n");
    return -1;
  }
  balance = (uint64_t)send_msg_args.balance->dval[0] * 1000000;

  if (balance > 0) {
    printf("send ");
    print_iota(balance);
    printf(" to %s\n", send_msg_args.receiver->sval[0]);
    printf("\n");
  } else {
    printf("send indexation payload to tangle");
  }

  err = wallet_send(wallet, (uint32_t)send_msg_args.sender->dval[0], recv + 1, balance, "ESP32\xF0\x9F\x8D\xBB",
                    (byte_t *)data, sizeof(data));
  if (err) {
    printf("send message failed\n");
    return -1;
  }

  return 0;
}

static void register_send_msg() {
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

//============= Public functions====================

void register_wallet_commands() {
  // system info
  register_free();
  register_heap();
  register_stack_info();
  register_version();
  register_restart();

  // wallet APIs
  register_get_balance();
  register_send_msg();
  register_get_address();
}

int init_wallet() {
  byte_t seed[IOTA_SEED_BYTES] = {};
  uint64_t balance = 0;

  if (config_validation() != 0) {
    return -1;
  }

  if (strcmp(CONFIG_WALLET_SEED, "random") == 0) {
    random_seed(seed);
  } else {
    hex2bin(CONFIG_WALLET_SEED, strlen(CONFIG_WALLET_SEED), seed, sizeof(seed));
  }

  wallet = wallet_create(seed, "m/44'/4218'/0'/0'");
  if (!wallet) {
    ESP_LOGE(TAG, "wallet create failed\n");
    return -1;
  }
  wallet_set_endpoint(wallet, APP_NODE_URL, APP_NODE_PORT);

  // TODO: get node info and update bech32HRP
  return 0;
}
