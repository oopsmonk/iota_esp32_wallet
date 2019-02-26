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
)
# trinary
set(COMMON_TRINARY
    ${COMMON_DIR}/trinary/add.c
    ${COMMON_DIR}/trinary/bct.c
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
    ${COMMON_DIR}/curl-p/const.c
    ${COMMON_DIR}/curl-p/curl_p.c
    ${COMMON_DIR}/curl-p/digest.c
)
set(COMMON_KERL
    ${COMMON_DIR}/kerl/bigint.c
    ${COMMON_DIR}/kerl/converter.c
    ${COMMON_DIR}/kerl/kerl.c
    ${COMMON_DIR}/kerl/hash.c
)
set(COMMON_HELPERS
    ${COMMON_DIR}/helpers/checksum.c
    ${COMMON_DIR}/helpers/digest.c
    ${COMMON_DIR}/helpers/pow.c
    ${COMMON_DIR}/helpers/sign.c
)
set(COMMON_MODEL
    ${COMMON_DIR}/model/bundle.c
    ${COMMON_DIR}/model/transaction.c
    ${COMMON_DIR}/model/transfer.c
)
set(COMMON_POW
    ${COMMON_DIR}/pow/pow.c
)
set(COMMON_SIGN
    ${COMMON_DIR}/sign/v1/iss_curl.c
    ${COMMON_DIR}/sign/v1/iss_kerl.c
)

set(COMMON_SRC
    ${COMMON_ERROR}
    ${COMMON_TRINARY}
    ${COMMON_CURL}
    ${COMMON_KERL}
    ${COMMON_HELPERS}
    ${COMMON_MODEL}
    ${COMMON_POW}
    ${COMMON_SIGN}
)

#json serialization
set(JSON_SERIALIZER_DIR ${CCLIENT_DIR}/serialization)
set(JSON_SERIALIZER
    ${JSON_SERIALIZER_DIR}/json/json_serializer.c
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
    ${API_REQUEST_DIR}/get_inclusion_state.c
    ${API_REQUEST_DIR}/get_transactions_to_approve.c
    ${API_REQUEST_DIR}/get_trytes.c
    ${API_REQUEST_DIR}/remove_neighbors.c
    ${API_REQUEST_DIR}/store_transactions.c
)
set(API_RESPONSE_DIR ${CCLIENT_DIR}/response)
set(API_RESPONSE
    ${API_RESPONSE_DIR}/add_neighbors.c
    ${API_RESPONSE_DIR}/attach_to_tangle.c
    ${API_RESPONSE_DIR}/check_consistency.c
    ${API_RESPONSE_DIR}/find_transactions.c
    ${API_RESPONSE_DIR}/get_balances.c
    ${API_RESPONSE_DIR}/get_inclusion_state.c
    ${API_RESPONSE_DIR}/get_neighbors.c
    ${API_RESPONSE_DIR}/get_node_info.c
    ${API_RESPONSE_DIR}/get_tips.c
    ${API_RESPONSE_DIR}/get_transactions_to_approve.c
    ${API_RESPONSE_DIR}/get_trytes.c
    ${API_RESPONSE_DIR}/remove_neighbors.c
)
# cclient 
set(CCLIENT
    ${JSON_SERIALIZER}
    ${API_REQUEST}
    ${API_RESPONSE}
    ${CCLIENT_DIR}/types/types.c
    ${CCLIENT_DIR}/iota_client_core_api.c
    ${CCLIENT_DIR}/iota_client_extended_api.c
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
)

# esp-idf compoments 
set(COMPONENT_PRIV_REQUIRES 
    json
)

register_component()
