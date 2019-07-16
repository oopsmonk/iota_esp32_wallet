#!/bin/bash
cd ./keccak
git checkout -f 87944d97ee18978aa0ea8486edbb7acb08a8564a -b 20190624 --quiet
if [ $? -eq 0 ]; then
    git apply ../keccak_58b20ec99f8a891913d8cf0ea350d05b6fb3ae41.patch
else
    echo "$0 failed..."
fi
