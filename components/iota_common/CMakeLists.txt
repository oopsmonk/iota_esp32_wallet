# iota_common for ESP32 platfrom

set(COMMONLIB_DIR iota_common)
set(UTILS_DIR ${COMMONLIB_DIR}/utils)
set(COMMON_DIR ${COMMONLIB_DIR}/common)
set(CRYPTO_DIR ${COMMON_DIR}/crypto)
set(HASH_CONTAINERS_DIR ${UTILS_DIR}/containers/hash)

set(COMPONENT_PRIV_INCLUDEDIRS ${COMMONLIB_DIR})
# common/errors
set(ERROR_SRC
    ${COMMON_DIR}/errors.c
)
# trinary
set(TRINARY_SRC
    ${COMMON_DIR}/trinary/add.c
    ${COMMON_DIR}/trinary/flex_trit.c
    ${COMMON_DIR}/trinary/ptrit_incr.c
    ${COMMON_DIR}/trinary/trit_byte.c
    ${COMMON_DIR}/trinary/trit_long.c
    ${COMMON_DIR}/trinary/trit_tryte.c
    ${COMMON_DIR}/trinary/tryte_ascii.c
    ${COMMON_DIR}/trinary/tryte_long.c
    ${COMMON_DIR}/trinary/tryte.c
)

# utils
set(UTILS_SRC
    ${UTILS_DIR}/time.c
    ${UTILS_DIR}/logger_helper.c
    ${UTILS_DIR}/char_buffer.c
    ${UTILS_DIR}/memset_safe.c
    ${UTILS_DIR}/input_validators.c
)
#hash container
set(HASH_CONTAINERS_SRC
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
set(CRYPTO_SRC
    ${CRYPTO_DIR}/curl-p/const.c
    ${CRYPTO_DIR}/curl-p/curl_p.c
    ${CRYPTO_DIR}/curl-p/digest.c
    ${CRYPTO_DIR}/kerl/bigint.c
    ${CRYPTO_DIR}/kerl/converter.c
    ${CRYPTO_DIR}/kerl/kerl.c
    ${CRYPTO_DIR}/kerl/hash.c
    ${CRYPTO_DIR}/iss/v1/iss_curl.c
    ${CRYPTO_DIR}/iss/v1/iss_kerl.c
    ${CRYPTO_DIR}/iss/normalize.c
)

set(HELPERS_SRC
    ${COMMON_DIR}/helpers/checksum.c
    ${COMMON_DIR}/helpers/digest.c
    ${COMMON_DIR}/helpers/sign.c
)
set(MODEL_SRC
    ${COMMON_DIR}/model/bundle.c
    ${COMMON_DIR}/model/transaction.c
    ${COMMON_DIR}/model/transfer.c
)

set(COMPONENT_SRCS
    ${ERROR_SRC}
    ${TRINARY_SRC}
    ${CRYPTO_SRC}
    ${HELPERS_SRC}
    ${MODEL_SRC}
    ${HASH_CONTAINERS_SRC}
    ${UTILS_SRC}
)

set(COMPONENT_ADD_INCLUDEDIRS ${CMAKE_CURRENT_LIST_DIR}/${COMMONLIB_DIR})
# local components
set(COMPONENT_REQUIRES
   embear_logger 
   uthash
   keccak
)

register_component()

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