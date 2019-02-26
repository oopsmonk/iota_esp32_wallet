set(COMPONENT_SRCS
    logger/src/logger.c
)

set(COMPONENT_ADD_INCLUDEDIRS "${CMAKE_CURRENT_LIST_DIR}/logger/include")

register_component()