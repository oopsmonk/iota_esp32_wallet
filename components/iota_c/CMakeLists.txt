set(IOTA_SRC_DIR "iota_c/src")

set(CRYPTO_SRCS "${IOTA_SRC_DIR}/crypto/iota_crypto.c")

set(CORE_SRCS
    "${IOTA_SRC_DIR}/core/seed.c"
    "${IOTA_SRC_DIR}/core/address.c"
    "${IOTA_SRC_DIR}/core/models/inputs/utxo_input.c"
    "${IOTA_SRC_DIR}/core/models/outputs/outputs.c"
    "${IOTA_SRC_DIR}/core/models/payloads/indexation.c"
    "${IOTA_SRC_DIR}/core/models/payloads/transaction.c"
    "${IOTA_SRC_DIR}/core/models/message.c"
    "${IOTA_SRC_DIR}/core/utils/bech32.c"
    "${IOTA_SRC_DIR}/core/utils/byte_buffer.c"
    "${IOTA_SRC_DIR}/core/utils/iota_str.c"
    "${IOTA_SRC_DIR}/core/utils/slip10.c")

set(CLIENT_SRCS
    "${IOTA_SRC_DIR}/client/network/http_esp32.c"
    "${IOTA_SRC_DIR}/client/api/json_utils.c"
    "${IOTA_SRC_DIR}/client/api/message_builder.c"
    "${IOTA_SRC_DIR}/client/api/message.c"
    "${IOTA_SRC_DIR}/client/api/v1/find_message.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_balance.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_health.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_message_children.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_message_metadata.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_message.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_node_info.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_output.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_outputs_from_address.c"
    "${IOTA_SRC_DIR}/client/api/v1/get_tips.c"
    "${IOTA_SRC_DIR}/client/api/v1/response_error.c"
    "${IOTA_SRC_DIR}/client/api/v1/send_message.c")

idf_component_register(
  SRCS
  ${CRYPTO_SRCS}
  ${CORE_SRCS}
  ${CLIENT_SRCS}
  INCLUDE_DIRS
  "iota_c/src"
  PRIV_REQUIRES
  uthash
  libsodium
  esp-tls
  esp_http_client
  json)

add_definitions(-DCRYPTO_USE_SODIUM)
