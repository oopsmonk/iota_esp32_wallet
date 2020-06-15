#pragma once

#include "common/defs.h"

#include "cclient/api/core/core_api.h"

#define WALLET_VERSION_MAJOR 0
#define WALLET_VERSION_MINOR 0
#define WALLET_VERSION_MICRO 1

#define APP_WALLET_VERSION      \
  VER_STR(WALLET_VERSION_MAJOR) \
  "." VER_STR(WALLET_VERSION_MINOR) "." VER_STR(WALLET_VERSION_MICRO)

// Register system commands
void register_wallet_commands();
void init_iota_client();
void destory_iota_client();
