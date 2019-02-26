#!/bin/bash
COMPONENTS_DIR="$PWD/components"
for COMPONECT in ${COMPONENTS_DIR}/*; do
    INIT_SCRIPT=${COMPONECT}/init.sh
    if [ -f ${INIT_SCRIPT} ]; then
        echo "init $COMPONECT"
        cd ${COMPONECT}
        /bin/bash ./init.sh
        cd -
    fi
done
