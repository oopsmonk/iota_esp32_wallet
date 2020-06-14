# # Ref 64
# set(COMPONENT_SRCS
#     keccak/lib/low/KeccakP-1600/Reference/KeccakP-1600-reference.c
#     keccak/lib/high/Keccak/KeccakSpongeWidth1600.c
#     keccak/lib/high/Keccak/FIPS202/KeccakHash.c
# )
#
# set(COMPONENT_ADD_INCLUDEDIRS
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/common
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600/Reference
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/high/Keccak
# )

# Inplace 32
set(COMPONENT_SRCS
    keccak/lib/low/KeccakP-1600/Inplace32BI/KeccakP-1600-inplace32BI.c
    keccak/lib/high/Keccak/KeccakSpongeWidth1600.c
    keccak/lib/high/Keccak/FIPS202/KeccakHash.c
)

set(COMPONENT_ADD_INCLUDEDIRS 
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/common
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/common
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600/Inplace32BI
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/high/Keccak
)

# # Inplace 32 T2 on 1
# set(COMPONENT_SRCS
#     keccak/lib/low/KeccakP-1600/Inplace32BI/KeccakP-1600-inplace32BI.c
#     keccak/lib/low/KeccakP-1600-times2/FallbackOn1/KeccakP-1600-times2-on1.c
#     keccak/lib/high/Keccak/KeccakSpongeWidth1600.c
#     keccak/lib/high/Keccak/FIPS202/KeccakHash.c
# )
#
# set(COMPONENT_ADD_INCLUDEDIRS
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/common
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/common
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600/Inplace32BI
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600-times2/FallbackOn1
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/high/Keccak
# )

# # Inplace 32 T8 on 1
# set(COMPONENT_SRCS
#     keccak/lib/low/KeccakP-1600/Inplace32BI/KeccakP-1600-inplace32BI.c
#     keccak/lib/low/KeccakP-1600-times8/FallbackOn1/KeccakP-1600-times8-on1.c
#     keccak/lib/high/Keccak/KeccakSpongeWidth1600.c
#     keccak/lib/high/Keccak/FIPS202/KeccakHash.c
# )
#
# set(COMPONENT_ADD_INCLUDEDIRS
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/common
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/common
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600/Inplace32BI
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600-times8/FallbackOn1
#     ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/high/Keccak
# )

register_component()