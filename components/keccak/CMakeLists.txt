set(COMPONENT_SRCS
    keccak/lib/low/KeccakP-1600/Reference/KeccakP-1600-reference.c
    keccak/lib/high/Keccak/KeccakSpongeWidth1600.c
    keccak/lib/high/Keccak/FIPS202/KeccakHash.c
)

set(COMPONENT_ADD_INCLUDEDIRS 
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/common
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/low/KeccakP-1600/Reference
    ${CMAKE_CURRENT_LIST_DIR}/keccak/lib/high/Keccak
)

register_component()