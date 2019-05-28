set(COMPONENT_SRCS
    main.c
    client_api.c
)

set(COMPONENT_ADD_INCLUDEDIRS ${CMAKE_CURRENT_LIST_DIR})

set(COMPONENT_PRIV_REQUIRES
   embear_logger
   uthash
   http_parser
   keccak
   iota_cclient
   mbedtls
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