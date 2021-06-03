// Copyright 2021 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define WALLET_VERSION_MAJOR 0
#define WALLET_VERSION_MINOR 3
#define WALLET_VERSION_MICRO 0

#define VER_STR0(s) #s
#define VER_STR(s) VER_STR0(s)

#define APP_WALLET_VERSION      \
  VER_STR(WALLET_VERSION_MAJOR) \
  "." VER_STR(WALLET_VERSION_MINOR) "." VER_STR(WALLET_VERSION_MICRO)

// Register system commands
void register_wallet_commands();
int init_wallet();
// timestamp in seconds
uint64_t timestamp();