set(COMPONENT_SRCS
    http_parser/http_parser.c
)

set(COMPONENT_ADD_INCLUDEDIRS "${CMAKE_CURRENT_LIST_DIR}/http_parser")

register_component()