#!/bin/bash
cd ./entangled
git checkout origin/cclient_esp32 -b cclient_esp32
cd -
/bin/bash ./gen_hash_container.sh

