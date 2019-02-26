#!/bin/bash
cd ./keccak
git checkout -f 58b20ec99f8a891913d8cf0ea350d05b6fb3ae41 --quiet
if [ $? -eq 0 ]; then
    git apply ../keccak_58b20ec99f8a891913d8cf0ea350d05b6fb3ae41.patch
else
    echo "$0 failed..."
fi
