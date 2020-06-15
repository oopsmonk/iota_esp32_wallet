
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
#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "wallet_system.h"

// iota cclient library
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "common/helpers/sign.h"
#include "utils/input_validators.h"

static const char *TAG = "wallet_system";

typedef struct {
  uint32_t depth;                 /*!< number of bundles to go back to determine the transactions for approval. */
  uint8_t mwm;                    /*!< Minimum Weight Magnitude for doing Proof-of-Work */
  uint8_t security;               /*!< security level of addresses, value could be 1,2,3. */
  iota_client_service_t *client;  /*!< iota client service */
  char seed[NUM_TRYTES_HASH + 1]; /*!< seed string*/
} iota_ctx_t;

static iota_ctx_t iota_ctx;

static char const *amazon_ca1_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

static void to_uppercase(char *sPtr, int nchar) {
  for (int i = 0; i < nchar; i++) {
    char *element = sPtr + i;
    *element = toupper((unsigned char)*element);
  }
}

// string data is not reset in argtable3
static void arg_str_reset(struct arg_str *parent) {
  for (int i = 0; i < parent->count; i++) {
    parent->sval[i] = "";
  }
  parent->count = 0;
}

/* 'version' command */
static int fn_get_version(int argc, char **argv) {
  esp_chip_info_t info;
  esp_chip_info(&info);
  printf("IDF Version:%s\r\n", esp_get_idf_version());
  printf("Chip info:\r\n");
  printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
  printf("\tcores:%d\r\n", info.cores);
  printf("\tfeature:%s%s%s%s%d%s\r\n", info.features & CHIP_FEATURE_WIFI_BGN ? ", 802.11bgn" : "",
         info.features & CHIP_FEATURE_BLE ? ", BLE" : "", info.features & CHIP_FEATURE_BT ? ", BT" : "",
         info.features & CHIP_FEATURE_EMB_FLASH ? ", Embedded-Flash:" : ", External-Flash:",
         spi_flash_get_chip_size() / (1024 * 1024), " MB");
  printf("\trevision number:%d\r\n", info.revision);
  printf("IOTA_COMMON_VERSION: %s, IOTA_CLIENT_VERSION: %s\n", IOTA_COMMON_VERSION, CCLIENT_VERSION);
  printf("APP_VERSION: %s\n", APP_WALLET_VERSION);
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
  destory_iota_client();
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

/* 'node_info' command */
static int fn_node_info(int argc, char **argv) {
  retcode_t ret = RC_ERROR;
  get_node_info_res_t *node_res = get_node_info_res_new();
  if (node_res == NULL) {
    printf("Error: OOM\n");
    return 0;
  }

  if ((ret = iota_client_get_node_info(iota_ctx.client, node_res)) == RC_OK) {
    printf("=== Node: %s:%d ===\n", iota_ctx.client->http.host, iota_ctx.client->http.port);
    printf("appName %s \n", get_node_info_res_app_name(node_res));
    printf("appVersion %s \n", get_node_info_res_app_version(node_res));

    printf("latestMilestone: ");
    flex_trit_print(node_res->latest_milestone, NUM_TRITS_HASH);
    printf("\n");

    printf("latestMilestoneIndex %u \n", node_res->latest_milestone_index);

    printf("latestSolidSubtangleMilestone: ");
    flex_trit_print(node_res->latest_solid_subtangle_milestone, NUM_TRITS_HASH);
    printf("\n");

    printf("latestSolidSubtangleMilestoneIndex %u \n", node_res->latest_solid_subtangle_milestone_index);
    printf("neighbors %d \n", node_res->neighbors);
    printf("packetsQueueSize %d \n", node_res->packets_queue_size);
    printf("time %" PRIu64 " \n", node_res->time);
    printf("tips %d \n", node_res->tips);
    printf("transactionsToRequest %d \n", node_res->transactions_to_request);
  } else {
    printf("Error: %s", error_2_string(ret));
  }

  get_node_info_res_free(&node_res);
  return 0;
}

static void register_node_info() {
  const esp_console_cmd_t node_info_cmd = {
      .command = "node_info",
      .help = "Get IOTA node info",
      .hint = NULL,
      .func = &fn_node_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&node_info_cmd));
}

/* 'node_info_set' command */
static struct {
  struct arg_str *url;
  struct arg_int *port;
  struct arg_int *is_https;
  struct arg_end *end;
} node_info_set_args;

static int fn_node_info_set(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&node_info_set_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, node_info_set_args.end, argv[0]);
    return -1;
  }
  char const *url = node_info_set_args.url->sval[0];
  int port = node_info_set_args.port->ival[0];
  int is_https = node_info_set_args.is_https->ival[0];

  printf("%s, port %d, https %d\n", url, port, is_https);
  iota_client_service_t *service = iota_client_core_init(url, port, is_https ? amazon_ca1_pem : NULL);
  if (service == NULL) {
    printf("Init iota client failed\n");
    return -1;
  }

  get_node_info_res_t *node_res = get_node_info_res_new();
  if (node_res == NULL) {
    printf("Error: OOM\n");
    return -1;
  }

  if (iota_client_get_node_info(service, node_res) == RC_OK) {
    iota_client_core_destroy(&iota_ctx.client);
    iota_ctx.client = service;
  } else {
    iota_client_core_destroy(&service);
  }

  get_node_info_res_free(&node_res);
  return 0;
}

static void register_node_info_set() {
  node_info_set_args.url = arg_str1(NULL, NULL, "<url>", "Node URL");
  node_info_set_args.port = arg_int1(NULL, NULL, "<port>", "port number");
  node_info_set_args.is_https = arg_int1(NULL, NULL, "<is_https>", "0 or 1");
  node_info_set_args.end = arg_end(5);
  node_info_set_args.url->hdr.resetfn = (arg_resetfn *)arg_str_reset;

  const esp_console_cmd_t node_info_set_cmd = {
      .command = "node_info_set",
      .help = "Sets node info",
      .hint = " <url> <port> <is_https (0|1)> ",
      .func = &fn_node_info_set,
      .argtable = &node_info_set_args,
  };

  ESP_ERROR_CHECK(esp_console_cmd_register(&node_info_set_cmd));
}

/* 'seed' command */
static int fn_get_seed(int argc, char **argv) {
  printf("%s\n", iota_ctx.seed);
  return 0;
}

static void register_get_seed() {
  const esp_console_cmd_t get_seed_cmd = {
      .command = "seed",
      .help = "Get the seed of this wallet",
      .hint = NULL,
      .func = &fn_get_seed,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_seed_cmd));
}

/* 'seed_set' command */
static struct {
  struct arg_str *seed;
  struct arg_end *end;
} seed_set_args;

static int fn_seed_set(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&seed_set_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, seed_set_args.end, argv[0]);
    return -1;
  }

  char *seed = (char *)seed_set_args.seed->sval[0];
  to_uppercase(seed, strlen(seed));
  if (!is_seed((tryte_t *)seed)) {
    printf("Invalid SEED hash(%d), expect %d\n", strlen(seed), NUM_TRYTES_HASH);
    return -1;
  }

  strncpy(iota_ctx.seed, seed, NUM_TRYTES_HASH);

  return 0;
}

static void register_seed_set() {
  seed_set_args.seed = arg_str1(NULL, NULL, "<seed>", "Hash with 81 trytes");
  seed_set_args.seed->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  seed_set_args.end = arg_end(2);
  const esp_console_cmd_t seed_set_cmd = {
      .command = "seed_set",
      .help = "Set the SEED hash of this wallet",
      .hint = " <seed>",
      .func = &fn_seed_set,
      .argtable = &seed_set_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&seed_set_cmd));
}

/* 'balance' command */
static struct {
  struct arg_str *address;
  struct arg_end *end;
} get_balance_args;

static int fn_get_balance(int argc, char **argv) {
  retcode_t ret_code = RC_OK;
  flex_trit_t tmp_address[FLEX_TRIT_SIZE_243];

  int nerrors = arg_parse(argc, argv, (void **)&get_balance_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_balance_args.end, argv[0]);
    return 1;
  }

  get_balances_req_t *balance_req = get_balances_req_new();
  get_balances_res_t *balance_res = get_balances_res_new();
  if (!balance_req || !balance_res) {
    ESP_LOGE(TAG, "Error: OOM");
    goto done;
  }

  int addr_count = get_balance_args.address->count;
  for (int i = 0; i < addr_count; i++) {
    tryte_t const *address_ptr = (tryte_t *)get_balance_args.address->sval[i];
    if (!is_address(address_ptr)) {
      printf("Invalid address\n");
      goto done;
    }
    if (flex_trits_from_trytes(tmp_address, NUM_TRITS_HASH, address_ptr, NUM_TRYTES_HASH, NUM_TRYTES_HASH) == 0) {
      printf("Err: converting flex_trit failed\n");
      goto done;
    }

    if (get_balances_req_address_add(balance_req, tmp_address) != RC_OK) {
      printf("Err: adding the hash to queue failed.\n");
      goto done;
    }
  }

  balance_req->threshold = 100;

  if ((ret_code = iota_client_get_balances(iota_ctx.client, balance_req, balance_res)) == RC_OK) {
    hash243_queue_entry_t *q_iter = NULL;
    size_t balance_cnt = get_balances_res_balances_num(balance_res);
    for (size_t i = 0; i < balance_cnt; i++) {
      printf("[%" PRIu64 "] ", get_balances_res_balances_at(balance_res, i));
      flex_trit_print(get_balances_req_address_get(balance_req, i), NUM_TRITS_HASH);
      printf("\n");
    }

    CDL_FOREACH(balance_res->references, q_iter) {
      printf("reference: ");
      flex_trit_print(q_iter->hash, NUM_TRITS_HASH);
      printf("\n");
    }
  }

done:
  get_balances_req_free(&balance_req);
  get_balances_res_free(&balance_res);
  return ret_code;
}

static void register_get_balance() {
  // get_balance_args.address = arg_str1(NULL, NULL, "<ADDRESS>", "An Address hash");
  get_balance_args.address = arg_strn(NULL, NULL, "<address...>", 1, 10, "Address hashes");
  get_balance_args.address->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  get_balance_args.end = arg_end(12);
  const esp_console_cmd_t get_balance_cmd = {
      .command = "balance",
      .help = "Get the balance from an addresses",
      .hint = " <address...>",
      .func = &fn_get_balance,
      .argtable = &get_balance_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_balance_cmd));
}

/* 'account' command */
static int fn_account_data(int argc, char **argv) {
  retcode_t ret = RC_OK;
  flex_trit_t seed[FLEX_TRIT_SIZE_243];

  if (flex_trits_from_trytes(seed, NUM_TRITS_ADDRESS, (const tryte_t *)iota_ctx.seed, NUM_TRYTES_ADDRESS,
                             NUM_TRYTES_ADDRESS) == 0) {
    ESP_LOGE(TAG, "converting flex_trit failed");
    return 1;
  }

  // init account data
  account_data_t account = {};
  account_data_init(&account);

  if ((ret = iota_client_get_account_data(iota_ctx.client, seed, 2, &account)) == RC_OK) {
#if 0  // dump transaction hashes
    size_t tx_count = hash243_queue_count(account.transactions);
    for (size_t i = 0; i < tx_count; i++) {
      printf("[%zu]: ", i);
      flex_trit_print(hash243_queue_at(account.transactions, i), NUM_TRITS_ADDRESS);
      printf("\n");
    }
    printf("transaction count %zu\n", tx_count);
#endif

    // dump balance
    printf("total balance: %" PRIu64 "\n", account.balance);

    // dump unused address
    printf("unused address: ");
    flex_trit_print(account.latest_address, NUM_TRITS_ADDRESS);
    printf("\n");

    // dump addresses
    size_t addr_count = hash243_queue_count(account.addresses);
    printf("address count %zu\n", addr_count);
    for (size_t i = 0; i < addr_count; i++) {
      printf("[%zu] ", i);
      flex_trit_print(hash243_queue_at(account.addresses, i), NUM_TRITS_ADDRESS);
      printf(" : %" PRIu64 "\n", account_data_get_balance(&account, i));
    }

    account_data_clear(&account);
  } else {
    ESP_LOGE(TAG, "Error: %s\n", error_2_string(ret));
    return 2;
  }
  return 0;
}

static void register_account_data() {
  const esp_console_cmd_t account_data_cmd = {
      .command = "account",
      .help = "Get account data",
      .hint = NULL,
      .func = &fn_account_data,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&account_data_cmd));
}

void convertToUpperCase(char *sPtr, int nchar) {
  for (int i = 0; i < nchar; i++) {
    char *element = sPtr + i;
    *element = toupper((unsigned char)*element);
  }
}

/* 'send' command */
static struct {
  struct arg_str *receiver;
  struct arg_str *value;
  struct arg_str *tag;
  struct arg_str *remainder;
  struct arg_str *message;
  struct arg_end *end;
} send_args;

static int fn_send(int argc, char **argv) {
  retcode_t ret_code = RC_OK;

  int nerrors = arg_parse(argc, argv, (void **)&send_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, send_args.end, argv[0]);
    return 1;
  }

  // check parameters
  char const *receiver = send_args.receiver->sval[0];
  char const *remainder = send_args.remainder->sval[0];
  char const *msg = send_args.message->sval[0];
  char const *tag = send_args.tag->sval[0];
  char *endptr = NULL;
  int64_t value = strtoll(send_args.value->sval[0], &endptr, 10);

  char padded_tag[NUM_TRYTES_TAG + 1];
  size_t tag_size = strlen(tag);
  convertToUpperCase((char *)tag, tag_size);
  for (size_t i = 0; i < NUM_TRYTES_TAG; i++) {
    if (i < tag_size) {
      padded_tag[i] = tag[i];
    } else {
      padded_tag[i] = '9';
    }
  }
  padded_tag[NUM_TRYTES_TAG] = '\0';

  printf("sending %lld to %s\n", value, receiver);
  printf("security %d, depth %d, MWM %d, tag [%s]\n", iota_ctx.security, iota_ctx.depth, iota_ctx.mwm,
         strlen(tag) ? padded_tag : "empty");
  printf("remainder [%s]\n", strlen(remainder) ? remainder : "empty");
  printf("message [%s]\n", strlen(msg) ? msg : "empty");

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  transfer_array_t *transfers = transfer_array_new();

  /* transfer setup */
  transfer_t tf = {};
  // seed
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  if (flex_trits_from_trytes(seed, NUM_TRITS_ADDRESS, (tryte_t const *)iota_ctx.seed, NUM_TRYTES_ADDRESS,
                             NUM_TRYTES_ADDRESS) == 0) {
    ESP_LOGE(TAG, "seed flex_trits convertion failed");
    goto done;
  }

  // receiver
  if (flex_trits_from_trytes(tf.address, NUM_TRITS_ADDRESS, (tryte_t const *)receiver, NUM_TRYTES_ADDRESS,
                             NUM_TRYTES_ADDRESS) == 0) {
    ESP_LOGE(TAG, "address flex_trits convertion failed");
    goto done;
  }

  // tag
  printf("tag: %s\n", padded_tag);
  if (flex_trits_from_trytes(tf.tag, NUM_TRITS_TAG, (tryte_t const *)padded_tag, NUM_TRYTES_TAG, NUM_TRYTES_TAG) == 0) {
    ESP_LOGE(TAG, "tag flex_trits convertion failed");
    goto done;
  }

  // value
  tf.value = value;

  // message (optional)
  transfer_message_set_string(&tf, msg);

  transfer_array_add(transfers, &tf);

  ret_code = iota_client_send_transfer(iota_ctx.client, seed, iota_ctx.security, iota_ctx.depth, iota_ctx.mwm, false,
                                       transfers, NULL, NULL, NULL, bundle);

  printf("send transaction: %s\n", error_2_string(ret_code));
  if (ret_code == RC_OK) {
    flex_trit_t const *bundle_hash = bundle_transactions_bundle_hash(bundle);
    printf("bundle hash: ");
    flex_trit_print(bundle_hash, NUM_TRITS_HASH);
    printf("\n");
  }

done:
  bundle_transactions_free(&bundle);
  transfer_message_free(&tf);
  transfer_array_free(transfers);

  return 0;
}

static void register_send() {
  send_args.receiver = arg_str1(NULL, NULL, "<RECEIVER>", "A receiver address");
  send_args.value = arg_str0("v", "value", "<VALUE>", "A token value");
  send_args.remainder = arg_str0("r", "remainder", "<REMAINDER>", "A remainder address");
  send_args.message = arg_str0("m", "message", "<MESSAGE>", "a message for this transaction");
  send_args.tag = arg_str0("t", "tag", "<TAG>", "A tag for this transaction");
  send_args.end = arg_end(12);
  // reset callbacks
  send_args.receiver->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  send_args.value->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  send_args.remainder->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  send_args.message->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  send_args.tag->hdr.resetfn = (arg_resetfn *)arg_str_reset;

  esp_console_cmd_t const send_cmd = {
      .command = "send",
      .help = "send value or data to the Tangle.\n\tex: send ADDRESSES -v=100",
      .hint = NULL,
      .func = &fn_send,
      .argtable = &send_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&send_cmd));
}

/* 'transactions' command */
static struct {
  struct arg_str *address;
  struct arg_end *end;
} get_transactions_args;

static int fn_get_transactions(int argc, char **argv) {
  retcode_t ret_code = RC_OK;
  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243];

  int nerrors = arg_parse(argc, argv, (void **)&get_transactions_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_transactions_args.end, argv[0]);
    return 1;
  }

  char const *address_ptr = get_transactions_args.address->sval[0];

  if (strlen(address_ptr) != HASH_LENGTH_TRYTE) {
    ESP_LOGE(TAG, "Invalid hash length!");
    return 2;
  }

  find_transactions_req_t *transactions_req = find_transactions_req_new();
  find_transactions_res_t *transactions_res = find_transactions_res_new();

  if (!transactions_req || !transactions_res) {
    ESP_LOGE(TAG, "Error: OOM");
    goto done;
  }

  if (flex_trits_from_trytes(tmp_hash, NUM_TRITS_HASH, (const tryte_t *)address_ptr, NUM_TRYTES_HASH,
                             NUM_TRYTES_HASH) == 0) {
    ESP_LOGE(TAG, "Error: converting flex_trit failed");
    goto done;
  }

  if ((hash243_queue_push(&transactions_req->addresses, tmp_hash)) != RC_OK) {
    ESP_LOGE(TAG, "Error: add a hash to queue failed.\n");
    goto done;
  }

  if (iota_client_find_transactions(iota_ctx.client, transactions_req, transactions_res) == RC_OK) {
    size_t count = hash243_queue_count(transactions_res->hashes);
    hash243_queue_t curr = transactions_res->hashes;
    for (size_t i = 0; i < count; i++) {
      printf("[%ld] ", (long int)i);
      flex_trit_print(curr->hash, NUM_TRITS_HASH);
      printf("\n");
      curr = curr->next;
    }
    printf("tx count = %ld\n", (long int)count);
  }

done:
  find_transactions_req_free(&transactions_req);
  find_transactions_res_free(&transactions_res);
  return ret_code;
}

static void register_get_transactions() {
  get_transactions_args.address = arg_str1(NULL, NULL, "<ADDRESS>", "An Address hash");
  get_transactions_args.address->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  get_transactions_args.end = arg_end(1);
  const esp_console_cmd_t get_transactions_cmd = {
      .command = "transactions",
      .help = "Get the transaction associate to an address (after last milestone)",
      .hint = NULL,
      .func = &fn_get_transactions,
      .argtable = &get_transactions_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_transactions_cmd));
}

/* 'gen_hash' command */
tryte_t const tryte_chars[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '9'};

static struct {
  struct arg_int *len;
  struct arg_end *end;
} gen_hash_args;

static int fn_gen_hash(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&gen_hash_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, gen_hash_args.end, argv[0]);
    return -1;
  }

  int len = gen_hash_args.len->ival[0];
  char *hash = (char *)malloc(sizeof(char) * (len + 1));
  if (hash == NULL) {
    ESP_LOGE(TAG, "Out of Memory\n");
    return -1;
  }

  srand(time(0));
  for (int i = 0; i < len; i++) {
    memset(hash + i, tryte_chars[rand() % 27], 1);
  }
  hash[len] = '\0';

  printf("Hash: %s\n", hash);
  free(hash);
  return 0;
}

static void register_gen_hash() {
  gen_hash_args.len = arg_int1(NULL, NULL, "<lenght>", "a length of the hash");
  gen_hash_args.end = arg_end(2);
  const esp_console_cmd_t gen_hash_cmd = {
      .command = "gen_hash",
      .help = "Genrating a hash with a given length.\n  `gen_hash 81` for a random SEED",
      .hint = " <length>",
      .func = &fn_gen_hash,
      .argtable = &gen_hash_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&gen_hash_cmd));
}

/* 'get_addresses' command */
static struct {
  struct arg_str *start_idx;
  struct arg_str *end_idx;
  struct arg_end *end;
} get_addresses_args;

static int fn_get_addresses(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_addresses_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_addresses_args.end, argv[0]);
    return -1;
  }

  char *endptr = NULL;
  uint64_t start_index = strtoll(get_addresses_args.start_idx->sval[0], &endptr, 10);
  if (endptr == get_addresses_args.start_idx->sval[0]) {
    ESP_LOGE(TAG, "No digits were found in start index\n");
    return -1;
  }
  uint64_t end_index = strtoll(get_addresses_args.end_idx->sval[0], &endptr, 10);
  if (endptr == get_addresses_args.end_idx->sval[0]) {
    ESP_LOGE(TAG, "No digits were found in end index\n");
    return -1;
  }
  if (end_index < start_index) {
    ESP_LOGE(TAG, "end index[%" PRIu64 "] should bigger or equal to start index[%" PRIu64 "]\n", start_index,
             end_index);
    return -1;
  }

  printf("Security level: %d\n", iota_ctx.security);
  // printf("get address %"PRId64" , %"PRId64"\n", start_index, end_index);
  while (start_index <= end_index) {
    char *addr = iota_sign_address_gen_trytes(iota_ctx.seed, start_index, iota_ctx.security);
    printf("[%" PRIu64 "] %s\n", start_index, addr);
    free(addr);
    start_index++;
  }

  return 0;
}

static void register_get_addresses() {
  get_addresses_args.start_idx = arg_str1(NULL, NULL, "<start>", "start index");
  get_addresses_args.end_idx = arg_str1(NULL, NULL, "<end>", "end index");
  get_addresses_args.start_idx->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  get_addresses_args.end_idx->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  get_addresses_args.end = arg_end(2);
  const esp_console_cmd_t get_addresses_cmd = {
      .command = "get_addresses",
      .help = "Gets addresses by index",
      .hint = " <start> <end>",
      .func = &fn_get_addresses,
      .argtable = &get_addresses_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_addresses_cmd));
}

/* 'get_bundle' command */
static struct {
  struct arg_str *tail;
  struct arg_end *end;
} get_bundle_args;

static int fn_get_bundle(int argc, char **argv) {
  retcode_t ret_code = RC_OK;
  flex_trit_t tmp_tail[FLEX_TRIT_SIZE_243];
  bundle_status_t bundle_status = BUNDLE_NOT_INITIALIZED;
  bundle_transactions_t *bundle = NULL;

  int nerrors = arg_parse(argc, argv, (void **)&get_bundle_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_bundle_args.end, argv[0]);
    return -1;
  }

  tryte_t const *tail_ptr = (tryte_t *)get_bundle_args.tail->sval[0];
  if (!is_address(tail_ptr)) {
    ESP_LOGE(TAG, "Invalid address\n");
    return -1;
  }

  bundle_transactions_new(&bundle);
  if (flex_trits_from_trytes(tmp_tail, NUM_TRITS_HASH, tail_ptr, NUM_TRYTES_HASH, NUM_TRYTES_HASH) == 0) {
    ESP_LOGE(TAG, "converting flex_trit failed.\n");
  } else {
    if ((ret_code = iota_client_get_bundle(iota_ctx.client, tmp_tail, bundle, &bundle_status)) == RC_OK) {
      if (bundle_status == BUNDLE_VALID) {
        printf("=== bundle status: %d ===\n", bundle_status);
        bundle_dump(bundle);
      } else {
        ESP_LOGE(TAG, "Invalid bundle: %d\n", bundle_status);
      }
    } else {
      ESP_LOGE(TAG, "%s\n", error_2_string(ret_code));
    }
  }

  bundle_transactions_free(&bundle);
  return ret_code;
}

static void register_get_bundle() {
  get_bundle_args.tail = arg_strn(NULL, NULL, "<tail>", 1, 10, "A tail hash");
  get_bundle_args.tail->hdr.resetfn = (arg_resetfn *)arg_str_reset;
  get_bundle_args.end = arg_end(4);
  const esp_console_cmd_t get_bundle_cmd = {
      .command = "get_bundle",
      .help = "Gets associated transactions from a tail hash",
      .hint = " <tail>",
      .func = &fn_get_bundle,
      .argtable = &get_bundle_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_bundle_cmd));
}

/* 'client_conf' command */
static int fn_client_conf(int argc, char **argv) {
  (void)argc;
  (void)argv;
  printf("MWM %d, Depth %d, Security %d\n", iota_ctx.mwm, iota_ctx.depth, iota_ctx.security);
  return 0;
}

static void register_client_conf() {
  const esp_console_cmd_t client_conf_cmd = {
      .command = "client_conf",
      .help = "Show client configurations",
      .hint = NULL,
      .func = &fn_client_conf,
      .argtable = NULL,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&client_conf_cmd));
}

/* 'client_conf_set' command */
static struct {
  struct arg_int *mwm;
  struct arg_int *depth;
  struct arg_int *security;
  struct arg_end *end;
} client_conf_set_args;

static int fn_client_conf_set(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&client_conf_set_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, client_conf_set_args.end, argv[0]);
    return -1;
  }

  int security = client_conf_set_args.security->ival[0];
  if (!is_security_level(security)) {
    ESP_LOGE(TAG, "Invalid Security level.\n");
    return -1;
  }

  iota_ctx.security = security;
  iota_ctx.mwm = client_conf_set_args.mwm->ival[0];
  iota_ctx.depth = client_conf_set_args.depth->ival[0];
  printf("Set: MWM %d, Depth %d, Security %d\n", iota_ctx.mwm, iota_ctx.depth, iota_ctx.security);

  return 0;
}

static void register_client_conf_set() {
  client_conf_set_args.mwm = arg_int1(NULL, NULL, "<mwm>", "9 for testnet, 14 for mainnet");
  client_conf_set_args.depth =
      arg_int1(NULL, NULL, "<depth>", "The depth as which Random Walk starts, 3 is a typicall value used by wallets.");
  client_conf_set_args.security =
      arg_int1(NULL, NULL, "<security>", "The security level of addresses, can be 1, 2 or 3");
  client_conf_set_args.end = arg_end(4);
  const esp_console_cmd_t client_conf_set_cmd = {
      .command = "client_conf_set",
      .help = "Set client configurations",
      .hint = " <mwm> <depth> <security>",
      .func = &fn_client_conf_set,
      .argtable = &client_conf_set_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&client_conf_set_cmd));
}

//============= Public functions====================

void register_wallet_commands() {
  // system info
  register_free();
  register_heap();
  register_stack_info();
  register_version();
  register_restart();

  // cclient APIs
  register_node_info();
  register_node_info_set();
  register_get_seed();
  register_seed_set();
  register_get_balance();
  register_account_data();
  register_send();
  register_get_transactions();
  register_gen_hash();
  register_get_addresses();
  register_get_bundle();
  register_client_conf();
  register_client_conf_set();
}

void init_iota_client() {
  iota_ctx.depth = CONFIG_IOTA_NODE_DEPTH;
  iota_ctx.mwm = CONFIG_IOTA_NODE_MWM;
  iota_ctx.security = 2;
  memcpy(iota_ctx.seed, CONFIG_IOTA_SEED, NUM_TRYTES_HASH);
  iota_ctx.seed[NUM_TRYTES_HASH] = '\0';

#ifdef CONFIG_IOTA_NODE_ENABLE_HTTPS
  iota_ctx.client = iota_client_core_init(CONFIG_IOTA_NODE_URL, CONFIG_IOTA_NODE_PORT, amazon_ca1_pem);
#else
  iota_ctx.client = iota_client_core_init(CONFIG_IOTA_NODE_URL, CONFIG_IOTA_NODE_PORT, NULL);
#endif

#ifdef CONFIG_CCLIENT_DEBUG
  logger_helper_init(LOGGER_DEBUG);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);
#endif
  ESP_LOGI(TAG, "IOTA_COMMON_VERSION: %s IOTA_CLIENT_VERSION: %s\n", IOTA_COMMON_VERSION, CCLIENT_VERSION);
}

void destory_iota_client() { iota_client_core_destroy(&iota_ctx.client); }