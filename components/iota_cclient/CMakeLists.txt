# CClient for ESP32 platfrom

set(ENTANGLED_DIR entangled)
set(UTILS_DIR ${ENTANGLED_DIR}/utils)
set(COMMON_DIR ${ENTANGLED_DIR}/common)
set(CCLIENT_DIR ${ENTANGLED_DIR}/cclient)
set(HASH_CONTAINERS_DIR ${COMPONENT_PATH}/${UTILS_DIR}/containers/hash)

set(COMPONENT_PRIV_INCLUDEDIRS ${ENTANGLED_DIR})
# common/errors
set(COMMON_ERROR 
    ${COMMON_DIR}/errors.c
)
# utils
set(UTILS
    ${UTILS_DIR}/time.c
    ${UTILS_DIR}/logger_helper.c
    ${UTILS_DIR}/handles/socket.c
    ${UTILS_DIR}/char_buffer.c
    ${UTILS_DIR}/memset_safe.c
)
# trinary
set(COMMON_TRINARY
    ${COMMON_DIR}/trinary/add.c
    ${COMMON_DIR}/trinary/flex_trit.c
    ${COMMON_DIR}/trinary/ptrit_incr.c
    ${COMMON_DIR}/trinary/trit_array.c
    ${COMMON_DIR}/trinary/trit_byte.c
    ${COMMON_DIR}/trinary/trit_long.c
    ${COMMON_DIR}/trinary/trit_ptrit.c
    ${COMMON_DIR}/trinary/trit_tryte.c
    ${COMMON_DIR}/trinary/tryte_ascii.c
    ${COMMON_DIR}/trinary/tryte_long.c
)
#http client
set(HTTP_CLIENT
    ${CCLIENT_DIR}/http/http.c
    ${CCLIENT_DIR}/service.c
)
#hash container
set(HASH_CONTAINERS
    ${HASH_CONTAINERS_DIR}/hash_array.c
    ${HASH_CONTAINERS_DIR}/hash27_queue.c
    ${HASH_CONTAINERS_DIR}/hash81_queue.c
    ${HASH_CONTAINERS_DIR}/hash243_queue.c
    ${HASH_CONTAINERS_DIR}/hash6561_queue.c
    ${HASH_CONTAINERS_DIR}/hash8019_queue.c
    ${HASH_CONTAINERS_DIR}/hash27_stack.c
    ${HASH_CONTAINERS_DIR}/hash81_stack.c
    ${HASH_CONTAINERS_DIR}/hash243_stack.c
    ${HASH_CONTAINERS_DIR}/hash6561_stack.c
    ${HASH_CONTAINERS_DIR}/hash8019_stack.c
)
# common curl
set(COMMON_CURL
    ${COMMON_DIR}/crypto/curl-p/const.c
    ${COMMON_DIR}/crypto/curl-p/curl_p.c
    ${COMMON_DIR}/crypto/curl-p/digest.c
)
set(COMMON_KERL
    ${COMMON_DIR}/crypto/kerl/bigint.c
    ${COMMON_DIR}/crypto/kerl/converter.c
    ${COMMON_DIR}/crypto/kerl/kerl.c
    ${COMMON_DIR}/crypto/kerl/hash.c
)
set(COMMON_HELPERS
    ${COMMON_DIR}/helpers/checksum.c
    ${COMMON_DIR}/helpers/digest.c
    ${COMMON_DIR}/helpers/sign.c
)
set(COMMON_MODEL
    ${COMMON_DIR}/model/bundle.c
    ${COMMON_DIR}/model/transaction.c
    ${COMMON_DIR}/model/transfer.c
)
set(COMMON_SIGN
    ${COMMON_DIR}/crypto/iss/v1/iss_curl.c
    ${COMMON_DIR}/crypto/iss/v1/iss_kerl.c
    ${COMMON_DIR}/crypto/iss/normalize.c
)

set(COMMON_SRC
    ${COMMON_ERROR}
    ${COMMON_TRINARY}
    ${COMMON_CURL}
    ${COMMON_KERL}
    ${COMMON_HELPERS}
    ${COMMON_MODEL}
    ${COMMON_SIGN}
)

#json serialization
set(JSON_SERIALIZER_JSON_DIR ${CCLIENT_DIR}/serialization/json)
set(JSON_SERIALIZER_JSON
    ${JSON_SERIALIZER_JSON_DIR}/add_neighbors.c
    ${JSON_SERIALIZER_JSON_DIR}/attach_to_tangle.c
    ${JSON_SERIALIZER_JSON_DIR}/broadcast_transactions.c
    ${JSON_SERIALIZER_JSON_DIR}/check_consistency.c
    ${JSON_SERIALIZER_JSON_DIR}/error.c
    ${JSON_SERIALIZER_JSON_DIR}/find_transactions.c
    ${JSON_SERIALIZER_JSON_DIR}/get_balances.c
    ${JSON_SERIALIZER_JSON_DIR}/get_inclusion_states.c
    ${JSON_SERIALIZER_JSON_DIR}/get_missing_transactions.c
    ${JSON_SERIALIZER_JSON_DIR}/get_neighbors.c
    ${JSON_SERIALIZER_JSON_DIR}/get_node_info.c
    ${JSON_SERIALIZER_JSON_DIR}/get_tips.c
    ${JSON_SERIALIZER_JSON_DIR}/get_transactions_to_approve.c
    ${JSON_SERIALIZER_JSON_DIR}/get_trytes.c
    ${JSON_SERIALIZER_JSON_DIR}/helpers.c
    ${JSON_SERIALIZER_JSON_DIR}/json_serializer.c
    ${JSON_SERIALIZER_JSON_DIR}/logger.c
    ${JSON_SERIALIZER_JSON_DIR}/remove_neighbors.c
    ${JSON_SERIALIZER_JSON_DIR}/store_transactions.c
    ${JSON_SERIALIZER_JSON_DIR}/were_addresses_spent_from.c
)
#request
set(API_REQUEST_DIR ${CCLIENT_DIR}/request)
set(API_REQUEST
    ${API_REQUEST_DIR}/add_neighbors.c
    ${API_REQUEST_DIR}/attach_to_tangle.c
    ${API_REQUEST_DIR}/broadcast_transactions.c
    ${API_REQUEST_DIR}/check_consistency.c
    ${API_REQUEST_DIR}/find_transactions.c
    ${API_REQUEST_DIR}/get_balances.c
    ${API_REQUEST_DIR}/get_inclusion_states.c
    ${API_REQUEST_DIR}/get_transactions_to_approve.c
    ${API_REQUEST_DIR}/get_trytes.c
    ${API_REQUEST_DIR}/remove_neighbors.c
    ${API_REQUEST_DIR}/store_transactions.c
    ${API_REQUEST_DIR}/were_addresses_spent_from.c
)
set(API_RESPONSE_DIR ${CCLIENT_DIR}/response)
set(API_RESPONSE
    ${API_RESPONSE_DIR}/add_neighbors.c
    ${API_RESPONSE_DIR}/attach_to_tangle.c
    ${API_RESPONSE_DIR}/check_consistency.c
    ${API_RESPONSE_DIR}/error.c
    ${API_RESPONSE_DIR}/find_transactions.c
    ${API_RESPONSE_DIR}/get_balances.c
    ${API_RESPONSE_DIR}/get_inclusion_states.c
    ${API_RESPONSE_DIR}/get_missing_transactions.c
    ${API_RESPONSE_DIR}/get_neighbors.c
    ${API_RESPONSE_DIR}/get_node_info.c
    ${API_RESPONSE_DIR}/get_tips.c
    ${API_RESPONSE_DIR}/get_transactions_to_approve.c
    ${API_RESPONSE_DIR}/get_trytes.c
    ${API_RESPONSE_DIR}/remove_neighbors.c
    ${API_RESPONSE_DIR}/were_addresses_spent_from.c
)

set(CCLIENT_API_CORE_DIR ${CCLIENT_DIR}/api/core)
set(CCLIENT_CORE
    ${CCLIENT_API_CORE_DIR}/get_inclusion_states.c
    ${CCLIENT_API_CORE_DIR}/get_node_info.c
    ${CCLIENT_API_CORE_DIR}/get_neighbors.c
    ${CCLIENT_API_CORE_DIR}/core_init.c
    ${CCLIENT_API_CORE_DIR}/get_transactions_to_approve.c
    ${CCLIENT_API_CORE_DIR}/find_transactions.c
    ${CCLIENT_API_CORE_DIR}/logger.c
    ${CCLIENT_API_CORE_DIR}/attach_to_tangle.c
    ${CCLIENT_API_CORE_DIR}/store_transactions.c
    ${CCLIENT_API_CORE_DIR}/get_balances.c
    ${CCLIENT_API_CORE_DIR}/remove_neighbors.c
    ${CCLIENT_API_CORE_DIR}/add_neighbors.c
    ${CCLIENT_API_CORE_DIR}/broadcast_transactions.c
    ${CCLIENT_API_CORE_DIR}/check_consistency.c
    ${CCLIENT_API_CORE_DIR}/get_tips.c
    ${CCLIENT_API_CORE_DIR}/get_trytes.c
    ${CCLIENT_API_CORE_DIR}/were_addresses_spent_from.c
)
set(CCLIENT_API_EXTENDED_DIR ${CCLIENT_DIR}/api/extended)
set(CCLIENT_EXTENDED
    ${CCLIENT_API_EXTENDED_DIR}/send_transfer.c
    ${CCLIENT_API_EXTENDED_DIR}/extended_init.c
    ${CCLIENT_API_EXTENDED_DIR}/get_account_data.c
    ${CCLIENT_API_EXTENDED_DIR}/broadcast_bundle.c
    ${CCLIENT_API_EXTENDED_DIR}/get_new_address.c
    ${CCLIENT_API_EXTENDED_DIR}/is_promotable.c
    ${CCLIENT_API_EXTENDED_DIR}/send_trytes.c
    ${CCLIENT_API_EXTENDED_DIR}/find_transaction_objects.c
    ${CCLIENT_API_EXTENDED_DIR}/traverse_bundle.c
    ${CCLIENT_API_EXTENDED_DIR}/logger.c
    ${CCLIENT_API_EXTENDED_DIR}/replay_bundle.c
    ${CCLIENT_API_EXTENDED_DIR}/store_and_broadcast.c
    ${CCLIENT_API_EXTENDED_DIR}/get_inputs.c
    ${CCLIENT_API_EXTENDED_DIR}/prepare_transfers.c
    ${CCLIENT_API_EXTENDED_DIR}/get_latest_inclusion.c
    ${CCLIENT_API_EXTENDED_DIR}/get_bundle.c
    ${CCLIENT_API_EXTENDED_DIR}/promote_transaction.c
    ${CCLIENT_API_EXTENDED_DIR}/get_transaction_objects.c
)
# cclient 
set(CCLIENT
    ${JSON_SERIALIZER_JSON}
    ${API_REQUEST}
    ${API_RESPONSE}
    ${CCLIENT_CORE}
    ${CCLIENT_EXTENDED}
)

set(COMPONENT_SRCS
    ${HASH_CONTAINERS}
    ${COMMON_SRC}
    ${UTILS}
    ${HTTP_CLIENT}
    ${CCLIENT}
)

set(COMPONENT_ADD_INCLUDEDIRS ${CMAKE_CURRENT_LIST_DIR}/${ENTANGLED_DIR})
# local components
set(COMPONENT_REQUIRES
   embear_logger 
   uthash
   http_parser
   keccak
   mbedtls
)

# esp-idf compoments 
set(COMPONENT_PRIV_REQUIRES 
    json
)

register_component()

# CClient debug
if(CONFIG_CCLIENT_DEBUG)
    add_definitions(-DDEBUG)
endif()

# flex_trit encoding
if(CONFIG_ONE_TRIT_PER_BYTE)
    add_definitions(-DFLEX_TRIT_ENCODING_1_TRITS_PER_BYTE)
elseif(CONFIG_THREE_TRIT_PER_BYTE)
    add_definitions(-DFLEX_TRIT_ENCODING_3_TRITS_PER_BYTE)
elseif(CONFIG_FOUR_TRIT_PER_BYTE)
    add_definitions(-DFLEX_TRIT_ENCODING_4_TRITS_PER_BYTE)
elseif(CONFIG_FIVE_TRIT_PER_BYTE)
    add_definitions(-DFLEX_TRIT_ENCODING_5_TRITS_PER_BYTE)
endif()