# Copyright 2020 IOTA Stiftung
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_IOTA_UNIT_TESTS)
    idf_component_register(SRCS "../test/test_main.c" INCLUDE_DIRS ".")
else()
    idf_component_register(SRCS "main.c" INCLUDE_DIRS ".")
endif()
