// Copyright 2020 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

#include <inttypes.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "sys/time.h"
#include "unity.h"

#include "core/models/message.h"
#include "core/models/payloads/transaction.h"

static const char* TAG = "test";

//===========Utils===========
static int64_t time_in_ms() {
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  return ((int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec) / 1000;
}

static int64_t time_in_us() {
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  return (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
};

//===========Tests===========
TEST_CASE("Address Generation", "[core]") {
  char const* const exp_iot_bech32 = "iot1qpg4tqh7vj9s7y9zk2smj8t4qgvse9um42l7apdkhw6syp5ju4w3v6ffg6n";
  char const* const exp_iota_bech32 = "iota1qpg4tqh7vj9s7y9zk2smj8t4qgvse9um42l7apdkhw6syp5ju4w3v79tf3l";
  byte_t exp_addr[IOTA_ADDRESS_BYTES] = {0x00, 0x51, 0x55, 0x82, 0xfe, 0x64, 0x8b, 0xf,  0x10, 0xa2, 0xb2,
                                         0xa1, 0xb9, 0x1d, 0x75, 0x2,  0x19, 0xc,  0x97, 0x9b, 0xaa, 0xbf,
                                         0xee, 0x85, 0xb6, 0xbb, 0xb5, 0x2,  0x6,  0x92, 0xe5, 0x5d, 0x16};
  byte_t exp_ed_addr[ED25519_ADDRESS_BYTES] = {0x4d, 0xbc, 0x7b, 0x45, 0x32, 0x46, 0x64, 0x20, 0x9a, 0xe5, 0x59,
                                               0xcf, 0xd1, 0x73, 0xc,  0xb,  0xb1, 0x90, 0x5a, 0x7f, 0x83, 0xe6,
                                               0x5d, 0x48, 0x37, 0xa9, 0x87, 0xe2, 0x24, 0xc1, 0xc5, 0x1e};

  byte_t seed[IOTA_SEED_BYTES] = {};
  byte_t addr_from_path[ED25519_ADDRESS_BYTES] = {};
  char bech32_addr[128] = {};
  byte_t addr_with_ver[IOTA_ADDRESS_BYTES] = {};
  byte_t addr_from_bech32[IOTA_ADDRESS_BYTES] = {};

  // convert seed from hex string to binary
  TEST_ASSERT(hex2bin("e57fb750f3a3a67969ece5bd9ae7eef5b2256a818b2aac458941f7274985a410", IOTA_SEED_BYTES * 2, seed,
                      IOTA_SEED_BYTES) == 0);

  TEST_ASSERT(address_from_path(seed, "m/44'/4218'/0'/0'/0'", addr_from_path) == 0);
  // dump_hex(addr_from_path, ED25519_ADDRESS_BYTES);

  // ed25519 address to IOTA address
  addr_with_ver[0] = ADDRESS_VER_ED25519;
  memcpy(addr_with_ver + 1, addr_from_path, ED25519_ADDRESS_BYTES);
  TEST_ASSERT_EQUAL_MEMORY(exp_addr, addr_with_ver, IOTA_ADDRESS_BYTES);
  // dump_hex(addr_with_ver, IOTA_ADDRESS_BYTES);

  // convert binary address to bech32 with iot HRP
  TEST_ASSERT(address_2_bech32(addr_with_ver, "iot", bech32_addr) == 0);
  TEST_ASSERT_EQUAL_STRING(exp_iot_bech32, bech32_addr);
  printf("bech32 [iot]: %s\n", bech32_addr);
  // bech32 to binary address
  TEST_ASSERT(address_from_bech32("iot", bech32_addr, addr_from_bech32) == 0);
  TEST_ASSERT_EQUAL_MEMORY(addr_with_ver, addr_from_bech32, IOTA_ADDRESS_BYTES);

  // convert binary address to bech32 with iota HRP
  TEST_ASSERT(address_2_bech32(addr_with_ver, "iota", bech32_addr) == 0);
  TEST_ASSERT_EQUAL_STRING(exp_iota_bech32, bech32_addr);
  printf("bech32 [iota]: %s\n", bech32_addr);
  // bech32 to binary address
  TEST_ASSERT(address_from_bech32("iota", bech32_addr, addr_from_bech32) == 0);
  TEST_ASSERT_EQUAL_MEMORY(addr_with_ver, addr_from_bech32, IOTA_ADDRESS_BYTES);

  // address from ed25519 keypair
  iota_keypair_t seed_keypair = {};
  byte_t ed_addr[ED25519_ADDRESS_BYTES] = {};

  // address from ed25519 public key
  iota_crypto_keypair(seed, &seed_keypair);
  TEST_ASSERT(address_from_ed25519_pub(seed_keypair.pub, ed_addr) == 0);
  TEST_ASSERT_EQUAL_MEMORY(exp_ed_addr, ed_addr, ED25519_ADDRESS_BYTES);
  // dump_hex(ed_addr, ED25519_ADDRESS_BYTES);
}

TEST_CASE("Message Signing", "[core]") {
  byte_t tx_id0[TRANSACTION_ID_BYTES] = {};
  byte_t addr0[ED25519_ADDRESS_BYTES] = {0x51, 0x55, 0x82, 0xfe, 0x64, 0x8b, 0x0f, 0x10, 0xa2, 0xb2, 0xa1,
                                         0xb9, 0x1d, 0x75, 0x02, 0x19, 0x0c, 0x97, 0x9b, 0xaa, 0xbf, 0xee,
                                         0x85, 0xb6, 0xbb, 0xb5, 0x02, 0x06, 0x92, 0xe5, 0x5d, 0x16};
  byte_t addr1[ED25519_ADDRESS_BYTES] = {0x69, 0x20, 0xb1, 0x76, 0xf6, 0x13, 0xec, 0x7b, 0xe5, 0x9e, 0x68,
                                         0xfc, 0x68, 0xf5, 0x97, 0xeb, 0x33, 0x93, 0xaf, 0x80, 0xf7, 0x4c,
                                         0x7c, 0x3d, 0xb7, 0x81, 0x98, 0x14, 0x7d, 0x5f, 0x1f, 0x92};

  iota_keypair_t seed_keypair = {};
  TEST_ASSERT(hex2bin("f7868ab6bb55800b77b8b74191ad8285a9bf428ace579d541fda47661803ff44", 64, seed_keypair.pub,
                      ED_PUBLIC_KEY_BYTES) == 0);
  TEST_ASSERT(hex2bin("256a818b2aac458941f7274985a410e57fb750f3a3a67969ece5bd9ae7eef5b2f7868ab6bb55800b77b8b74191ad8285"
                      "a9bf428ace579d541fda47661803ff44",
                      128, seed_keypair.priv, ED_PRIVATE_KEY_BYTES) == 0);

  core_message_t* msg = core_message_new();
  TEST_ASSERT_NOT_NULL(msg);

  transaction_payload_t* tx = tx_payload_new();
  TEST_ASSERT_NOT_NULL(tx);

  TEST_ASSERT(tx_payload_add_input_with_key(tx, tx_id0, 0, seed_keypair.pub, seed_keypair.priv) == 0);
  TEST_ASSERT(tx_payload_add_output(tx, OUTPUT_SINGLE_OUTPUT, addr0, 1000) == 0);
  TEST_ASSERT(tx_payload_add_output(tx, OUTPUT_SINGLE_OUTPUT, addr1, 2779530283276761) == 0);

  // put tx payload into message
  msg->payload_type = 0;
  msg->payload = tx;

  TEST_ASSERT(core_message_sign_transaction(msg) == 0);

  // tx_payload_print(tx);

  // free message and sub entities
  core_message_free(msg);
}

TEST_CASE("Essence Serialization", "[core]") {
  byte_t exp_essence_byte[128] = {
      0x0,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
      0x2,  0x0,  0x0,  0x0,  0x51, 0x55, 0x82, 0xFE, 0x64, 0x8B, 0xF,  0x10, 0xA2, 0xB2, 0xA1, 0xB9, 0x1D, 0x75, 0x2,
      0x19, 0xC,  0x97, 0x9B, 0xAA, 0xBF, 0xEE, 0x85, 0xB6, 0xBB, 0xB5, 0x2,  0x6,  0x92, 0xE5, 0x5D, 0x16, 0xE8, 0x3,
      0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x69, 0x20, 0xB1, 0x76, 0xF6, 0x13, 0xEC, 0x7B, 0xE5, 0x9E, 0x68,
      0xFC, 0x68, 0xF5, 0x97, 0xEB, 0x33, 0x93, 0xAF, 0x80, 0xF7, 0x4C, 0x7C, 0x3D, 0xB7, 0x81, 0x98, 0x14, 0x7D, 0x5F,
      0x1F, 0x92, 0xD9, 0x59, 0x2D, 0xD3, 0xF7, 0xDF, 0x9,  0x0,  0x0,  0x0,  0x0,  0x0};
  static byte_t addr_a[ED25519_ADDRESS_BYTES] = {0x51, 0x55, 0x82, 0xfe, 0x64, 0x8b, 0x0f, 0x10, 0xa2, 0xb2, 0xa1,
                                                 0xb9, 0x1d, 0x75, 0x02, 0x19, 0x0c, 0x97, 0x9b, 0xaa, 0xbf, 0xee,
                                                 0x85, 0xb6, 0xbb, 0xb5, 0x02, 0x06, 0x92, 0xe5, 0x5d, 0x16};
  static byte_t addr_b[ED25519_ADDRESS_BYTES] = {0x69, 0x20, 0xb1, 0x76, 0xf6, 0x13, 0xec, 0x7b, 0xe5, 0x9e, 0x68,
                                                 0xfc, 0x68, 0xf5, 0x97, 0xeb, 0x33, 0x93, 0xaf, 0x80, 0xf7, 0x4c,
                                                 0x7c, 0x3d, 0xb7, 0x81, 0x98, 0x14, 0x7d, 0x5f, 0x1f, 0x92};
  static byte_t tx_id_empty[TRANSACTION_ID_BYTES] = {};

  transaction_essence_t* essence = tx_essence_new();
  TEST_ASSERT_NOT_NULL(essence);
  TEST_ASSERT(tx_essence_serialize_length(essence) == 0);

  // add inputs
  TEST_ASSERT(tx_essence_add_input(essence, tx_id_empty, 0) == 0);
  TEST_ASSERT_EQUAL_UINT32(1, utxo_inputs_count(&essence->inputs));

  // add outputs
  TEST_ASSERT(tx_essence_add_output(essence, OUTPUT_SINGLE_OUTPUT, addr_a, 1000) == 0);
  TEST_ASSERT(tx_essence_add_output(essence, OUTPUT_SINGLE_OUTPUT, addr_b, 2779530283276761) == 0);
  TEST_ASSERT_EQUAL_UINT32(2, utxo_outputs_count(&essence->outputs));

  // get serialized length
  size_t essence_buf_len = tx_essence_serialize_length(essence);
  TEST_ASSERT(essence_buf_len != 0);

  byte_t* essence_buf = malloc(essence_buf_len);
  TEST_ASSERT_NOT_NULL(essence_buf);
  TEST_ASSERT(tx_essence_serialize(essence, essence_buf) == essence_buf_len);
  // dump_hex(essence_buf, essence_buf_len);
  TEST_ASSERT_EQUAL_MEMORY(exp_essence_byte, essence_buf, sizeof(exp_essence_byte));
  free(essence_buf);
  tx_essence_free(essence);
}

static byte_t test_pub_key[ED_PUBLIC_KEY_BYTES] = {0xe7, 0x45, 0x3d, 0x64, 0x4d, 0x7b, 0xe6, 0x70, 0x64, 0x80, 0x15,
                                                   0x74, 0x28, 0xd9, 0x68, 0x87, 0x2e, 0x38, 0x9c, 0x7b, 0x27, 0x62,
                                                   0xd1, 0x4b, 0xbe, 0xc,  0xa4, 0x6b, 0x91, 0xde, 0xa4, 0xc4};
static byte_t test_sig[ED_SIGNATURE_BYTES] = {
    0x74, 0x9,  0x52, 0x4c, 0xa4, 0x4,  0xfb, 0x5e, 0x51, 0xe3, 0xc6, 0x65, 0xf1, 0x1f, 0xa6, 0x61,
    0x4,  0xc3, 0xe,  0x8,  0xe9, 0x0,  0x38, 0x4f, 0xdd, 0xeb, 0x5b, 0x93, 0xb6, 0xed, 0xa0, 0x54,
    0xc5, 0x3,  0x3e, 0xbd, 0xd4, 0xd8, 0xa7, 0xa,  0x7b, 0xa8, 0xbb, 0xcc, 0x7a, 0x34, 0x4d, 0x56,
    0xe2, 0xba, 0x11, 0xd2, 0x2a, 0xf3, 0xab, 0xe4, 0x6e, 0x99, 0x21, 0x56, 0x25, 0x73, 0xf2, 0x62};
static byte_t exp_block[103] = {
    0x2,  0x0,  0x0,  0x1,  0xE7, 0x45, 0x3D, 0x64, 0x4D, 0x7B, 0xE6, 0x70, 0x64, 0x80, 0x15, 0x74, 0x28, 0xD9,
    0x68, 0x87, 0x2E, 0x38, 0x9C, 0x7B, 0x27, 0x62, 0xD1, 0x4B, 0xBE, 0xC,  0xA4, 0x6B, 0x91, 0xDE, 0xA4, 0xC4,
    0x74, 0x9,  0x52, 0x4C, 0xA4, 0x4,  0xFB, 0x5E, 0x51, 0xE3, 0xC6, 0x65, 0xF1, 0x1F, 0xA6, 0x61, 0x4,  0xC3,
    0xE,  0x8,  0xE9, 0x0,  0x38, 0x4F, 0xDD, 0xEB, 0x5B, 0x93, 0xB6, 0xED, 0xA0, 0x54, 0xC5, 0x3,  0x3E, 0xBD,
    0xD4, 0xD8, 0xA7, 0xA,  0x7B, 0xA8, 0xBB, 0xCC, 0x7A, 0x34, 0x4D, 0x56, 0xE2, 0xBA, 0x11, 0xD2, 0x2A, 0xF3,
    0xAB, 0xE4, 0x6E, 0x99, 0x21, 0x56, 0x25, 0x73, 0xF2, 0x62, 0x1,  0x0,  0x0};

TEST_CASE("Transaction Block Serialization", "[core]") {
  tx_unlock_blocks_t* blocks = tx_blocks_new();
  TEST_ASSERT_NULL(blocks);
  TEST_ASSERT_EQUAL_UINT16(0, tx_blocks_count(blocks));

  // add a signature block
  ed25519_signature_t sig = {};
  sig.type = 1;
  memcpy(sig.pub_key, test_pub_key, ED_PUBLIC_KEY_BYTES);
  memcpy(sig.signature, test_sig, ED_SIGNATURE_BYTES);
  tx_blocks_add_signature(&blocks, &sig);
  TEST_ASSERT_EQUAL_UINT16(1, tx_blocks_count(blocks));

  // add a reference block that reverence to the 0 index of blocks.
  tx_blocks_add_reference(&blocks, 0);

  // tx_blocks_print(blocks);

  // serialization
  size_t len = tx_blocks_serialize_length(blocks);
  TEST_ASSERT(len != 0);
  byte_t* block_buf = malloc(len);
  TEST_ASSERT(tx_blocks_serialize(blocks, block_buf) == len);
  TEST_ASSERT_EQUAL_MEMORY(exp_block, block_buf, sizeof(exp_block));
  // dump_hex(block_buf, len);

  free(block_buf);

  tx_blocks_free(blocks);
}

static byte_t addr0[ED25519_ADDRESS_BYTES] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static byte_t tx_id0[TRANSACTION_ID_BYTES] = {126, 127, 95,  249, 151, 44,  243, 150, 40,  39, 46,
                                              190, 54,  49,  73,  171, 165, 88,  139, 221, 25, 199,
                                              90,  172, 252, 142, 91,  179, 113, 2,   177, 58};
static byte_t exp_serialized_tx[] = {
    0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x0,  0x7E, 0x7F, 0x5F, 0xF9, 0x97, 0x2C, 0xF3, 0x96, 0x28, 0x27, 0x2E,
    0xBE, 0x36, 0x31, 0x49, 0xAB, 0xA5, 0x58, 0x8B, 0xDD, 0x19, 0xC7, 0x5A, 0xAC, 0xFC, 0x8E, 0x5B, 0xB3, 0x71, 0x2,
    0xB1, 0x3A, 0x0,  0x0,  0x1,  0x0,  0x0,  0x0,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0xE8, 0x3,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x0,  0x0,  0x0,  0xE7,
    0x45, 0x3D, 0x64, 0x4D, 0x7B, 0xE6, 0x70, 0x64, 0x80, 0x15, 0x74, 0x28, 0xD9, 0x68, 0x87, 0x2E, 0x38, 0x9C, 0x7B,
    0x27, 0x62, 0xD1, 0x4B, 0xBE, 0xC,  0xA4, 0x6B, 0x91, 0xDE, 0xA4, 0xC4, 0x74, 0x9,  0x52, 0x4C, 0xA4, 0x4,  0xFB,
    0x5E, 0x51, 0xE3, 0xC6, 0x65, 0xF1, 0x1F, 0xA6, 0x61, 0x4,  0xC3, 0xE,  0x8,  0xE9, 0x0,  0x38, 0x4F, 0xDD, 0xEB,
    0x5B, 0x93, 0xB6, 0xED, 0xA0, 0x54, 0xC5, 0x3,  0x3E, 0xBD, 0xD4, 0xD8, 0xA7, 0xA,  0x7B, 0xA8, 0xBB, 0xCC, 0x7A,
    0x34, 0x4D, 0x56, 0xE2, 0xBA, 0x11, 0xD2, 0x2A, 0xF3, 0xAB, 0xE4, 0x6E, 0x99, 0x21, 0x56, 0x25, 0x73, 0xF2, 0x62};

TEST_CASE("Payload Serialization", "[core]") {
  transaction_payload_t* tx = tx_payload_new();
  TEST_ASSERT_NOT_NULL(tx);

  // add input
  TEST_ASSERT(tx_payload_add_input(tx, tx_id0, 0) == 0);
  // add output
  TEST_ASSERT(tx_payload_add_output(tx, OUTPUT_SINGLE_OUTPUT, addr0, 1000) == 0);

  // add signature
  ed25519_signature_t sig = {};
  sig.type = ADDRESS_VER_ED25519;
  memcpy(sig.pub_key, test_pub_key, ED_PUBLIC_KEY_BYTES);
  memcpy(sig.signature, test_sig, ED_SIGNATURE_BYTES);
  TEST_ASSERT(tx_payload_add_sig_block(tx, &sig) == 0);

  // tx_payload_print(tx);

  // serialization
  size_t tx_len = tx_payload_serialize_length(tx);
  TEST_ASSERT(tx_len != 0);
  byte_t* tx_buf = malloc(tx_len);
  TEST_ASSERT(tx_payload_serialize(tx, tx_buf) == tx_len);
  TEST_ASSERT_EQUAL_MEMORY(exp_serialized_tx, tx_buf, sizeof(exp_serialized_tx));
  // dump_hex(tx_buf, tx_len);
  free(tx_buf);
  tx_payload_free(tx);
}

//=========Benchmarks========
#define ADDR_NUMS 100

TEST_CASE("Bench address generation", "[bench]") {
  char path_buf[128] = {};
  byte_t seed[IOTA_SEED_BYTES] = {};
  byte_t ed_addr[IOTA_ADDRESS_BYTES] = {};
  char bech32_addr[IOTA_ADDRESS_HEX_BYTES + 1] = {};
  size_t ret_size = 0;
  int64_t start_time = 0, time_spent = 0;
  int64_t min = 0, max = 0, sum = 0;
  random_seed(seed);

  for (size_t idx = 0; idx < ADDR_NUMS; idx++) {
    ret_size = snprintf(path_buf, 128, "m/44'/4218'/0'/0'/%zu'", idx);
    if (ret_size >= 128) {
      path_buf[128 - 1] = '\0';
    }
    ed_addr[0] = 0;
    start_time = time_in_us();
    if (address_from_path(seed, path_buf, ed_addr + 1) == 0) {
      if (address_2_bech32(ed_addr, "iota", bech32_addr) != 0) {
        printf("convert to bech32 failed\n");
        break;
      }
      time_spent = time_in_us() - start_time;
      max = (idx == 0 || time_spent > max) ? time_spent : max;
      min = (idx == 0 || time_spent < min) ? time_spent : min;
      sum += time_spent;
      // printf("%zu: %"PRId64", max %"PRId64", min %"PRId64"\n", idx, time_spent, max, min);
    } else {
      printf("drive from path failed\n");
      break;
    }
  }
  printf("Bench %d address generation\n\tmin(ms)\tmax(ms)\tavg(ms)\ttotal(ms)\n", ADDR_NUMS);
  printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", (min / 1000.0), (max / 1000.0), (sum / ADDR_NUMS) / 1000.0, sum / 1000.0);
}

void app_main(void) {
  printf("===============================\n");
  printf("=====Unit Test Application=====\n");
  printf("===============================\n");

  /* Print chip information */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), WiFi%s%s, ", CONFIG_IDF_TARGET, chip_info.cores,
         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

  UNITY_BEGIN();
  ESP_LOGI(TAG, "%s Testing...", CONFIG_IDF_TARGET);
  unity_run_tests_by_tag("[bench]", true);
  UNITY_END();

  UNITY_BEGIN();
  ESP_LOGI(TAG, "%s Benchmarking...", CONFIG_IDF_TARGET);
  unity_run_tests_by_tag("[bench]", false);
  UNITY_END();

  /* This function will not return, and will be busy waiting for UART input.
   * Make sure that task watchdog is disabled if you use this function.
   */
  unity_run_menu();
}