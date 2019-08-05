set(COMPONENT_SRCS
    main.c
    wallet_system.c
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

set(COMPONENT_REQUIRES console spi_flash nvs_flash)

register_component()
