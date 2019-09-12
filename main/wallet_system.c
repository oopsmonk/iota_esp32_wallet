
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
// #include "esp_sleep.h"
#include "argtable3/argtable3.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/uart.h"
#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "wallet_system.h"

// iota cclient library
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"

static const char *TAG = "wallet_system";

static iota_client_service_t g_cclient;

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
  printf("IOTA CClient Version:%s\r\n", "v1.0.0-beta");
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

/* 'info' command */
static int fn_node_info(int argc, char **argv) {
  retcode_t ret = RC_ERROR;
  get_node_info_res_t *node_res = get_node_info_res_new();
  if (node_res == NULL) {
    printf("Error: OOM\n");
    return 0;
  }

  if ((ret = iota_client_get_node_info(&g_cclient, node_res)) == RC_OK) {
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
      .command = "info",
      .help = "Get IRI node info",
      .hint = NULL,
      .func = &fn_node_info,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&node_info_cmd));
}

/* 'seed' command */
static int fn_get_seed(int argc, char **argv) {
  printf("%s\n", CONFIG_IOTA_SEED);
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

/* 'balance' command */
static struct {
  struct arg_str *address;
  struct arg_end *end;
} get_balance_args;

static int fn_get_balance(int argc, char **argv) {
  retcode_t ret_code = RC_OK;
  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243];

  int nerrors = arg_parse(argc, argv, (void **)&get_balance_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_balance_args.end, argv[0]);
    return 1;
  }

  char const *address_ptr = get_balance_args.address->sval[0];

#if 0
  if(argc != 2){
    ESP_LOGE(TAG, "Invalid input");
    return 2;
  }

#endif
  if (strlen(address_ptr) != HASH_LENGTH_TRYTE) {
    ESP_LOGE(TAG, "Invalid hash length!");
    return 2;
  }

  get_balances_req_t *balance_req = get_balances_req_new();
  get_balances_res_t *balance_res = get_balances_res_new();

  if (!balance_req || !balance_res) {
    ESP_LOGE(TAG, "Error: OOM");
    goto done;
  }

  if (flex_trits_from_trytes(tmp_hash, NUM_TRITS_HASH, (const tryte_t *)address_ptr, NUM_TRYTES_HASH,
                             NUM_TRYTES_HASH) == 0) {
    ESP_LOGE(TAG, "Error: converting flex_trit failed");
    goto done;
  }

  if ((ret_code = get_balances_req_address_add(balance_req, tmp_hash)) != RC_OK) {
    ESP_LOGE(TAG, "Error: Adding hash to list failed!\n");
    goto done;
  }

  balance_req->threshold = 100;

  if ((ret_code = iota_client_get_balances(&g_cclient, balance_req, balance_res)) == RC_OK) {
    hash243_queue_entry_t *q_iter = NULL;
    size_t balance_cnt = get_balances_res_balances_num(balance_res);
    printf("balances: [");
    for (size_t i = 0; i < balance_cnt; i++) {
      printf(" %" PRIu64 " ", get_balances_res_balances_at(balance_res, i));
    }
    printf("]\n");

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
  get_balance_args.address = arg_str1(NULL, NULL, "<ADDRESS>", "An Address hash");
  get_balance_args.address->hdr.resetfn = (arg_resetfn*)arg_str_reset;
  get_balance_args.end = arg_end(1);
  const esp_console_cmd_t get_balance_cmd = {
      .command = "balance",
      .help = "Get the balance from an address",
      .hint = NULL,
      .func = &fn_get_balance,
      .argtable = &get_balance_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&get_balance_cmd));
}

/* 'account' command */
static int fn_account_data(int argc, char **argv) {
  retcode_t ret = RC_OK;
  flex_trit_t seed[FLEX_TRIT_SIZE_243];

  if (flex_trits_from_trytes(seed, NUM_TRITS_ADDRESS, (const tryte_t *)CONFIG_IOTA_SEED, NUM_TRYTES_ADDRESS,
                             NUM_TRYTES_ADDRESS) == 0) {
    ESP_LOGE(TAG, "converting flex_trit failed");
    return 1;
  }

  // init account data
  account_data_t account = {};
  account_data_init(&account);

  if ((ret = iota_client_get_account_data(&g_cclient, seed, 2, &account)) == RC_OK) {
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

/* 'send' command */
static struct {
  struct arg_str *receiver;
  struct arg_str *value;
  struct arg_str *tag;
  struct arg_str *remainder;
  struct arg_str *message;
  struct arg_int *security;
  struct arg_end *end;
} send_args;

static int fn_send(int argc, char **argv) {
  retcode_t ret_code = RC_OK;
  flex_trit_t tmp_hash[FLEX_TRIT_SIZE_243];

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
  uint8_t security = send_args.security->ival[0];

  if (!(security > 0 && security <= 3)) {
    security = 2;
  }

  printf("sending %lld to %s\n", value, receiver);
  printf("security %d, depth %d, MWM %d, tag [%s]\n", security, CONFIG_IOTA_DEPTH, CONFIG_IOTA_MWM,
         strlen(tag) ? tag : "empty");
  printf("remainder [%s]\n", strlen(remainder) ? remainder : "empty");
  printf("message [%s]\n", strlen(msg) ? msg : "empty");

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  transfer_array_t *transfers = transfer_array_new();

  /* transfer setup */
  transfer_t tf = {};
  // seed
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  if (flex_trits_from_trytes(seed, NUM_TRITS_ADDRESS, (tryte_t const *)CONFIG_IOTA_SEED, NUM_TRYTES_ADDRESS,
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
  if (flex_trits_from_trytes(tf.tag, NUM_TRITS_TAG, (tryte_t const *)"CCLIENT99999999999999999999", NUM_TRYTES_TAG,
                             NUM_TRYTES_TAG) == 0) {
    ESP_LOGE(TAG, "tag flex_trits convertion failed");
    goto done;
  }

  // value
  tf.value = value;

  // message (optional)
  transfer_message_set_string(&tf, msg);

  transfer_array_add(transfers, &tf);

  ret_code = iota_client_send_transfer(&g_cclient, seed, security, CONFIG_IOTA_DEPTH, CONFIG_IOTA_MWM, false, transfers,
                                       NULL, NULL, NULL, bundle);

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
  send_args.security = arg_int0("s", "security", "<SECURITY>", "The security level of the address");
  send_args.end = arg_end(1);
  //reset callbacks
  send_args.receiver->hdr.resetfn = (arg_resetfn*)arg_str_reset;
  send_args.value->hdr.resetfn = (arg_resetfn*)arg_str_reset;
  send_args.remainder->hdr.resetfn = (arg_resetfn*)arg_str_reset;
  send_args.message->hdr.resetfn = (arg_resetfn*)arg_str_reset;
  send_args.tag->hdr.resetfn = (arg_resetfn*)arg_str_reset;

  esp_console_cmd_t const send_cmd = {
      .command = "send",
      .help = "send value or data to the Tangle.\n\tex: send ADDRESSES -v=100",
      .hint = NULL,
      .func = &fn_send,
      .argtable = &send_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&send_cmd));
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
  register_get_seed();
  register_get_balance();
  register_account_data();
  register_send();
}

void init_iota_client() {
  g_cclient.http.path = "/";
  g_cclient.http.content_type = "application/json";
  g_cclient.http.accept = "application/json";
  g_cclient.http.host = CONFIG_IRI_NODE_URI;
  g_cclient.http.port = CONFIG_IRI_NODE_PORT;
  g_cclient.http.api_version = 1;
#ifdef CONFIG_ENABLE_HTTPS
  g_cclient.http.ca_pem = amazon_ca1_pem;
#else
  g_cclient.http.ca_pem = NULL;
#endif
  g_cclient.serializer_type = SR_JSON;
  // logger_helper_init(LOGGER_DEBUG);
  iota_client_core_init(&g_cclient);
  iota_client_extended_init();
}