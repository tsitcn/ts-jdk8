#!/bin/bash

. ./lib_tsjdk8.sh

get_cups_dir

if [ -d    ${CUPS_DIR} ]; then
    rm_dir ${CUPS_DIR}
fi

tar -xzf ${CUPS_DIR}.tar.gz
copy_config_guess ${CUPS_DIR}

cd ${CUPS_DIR}

./configure \
    --with-tls=no

make all

if [ -d bin ]; then
    rm bin/*
else
    mkdir bin
fi

for file in lpr lprm lpq lpc
do
    file=berkeley/${file}
    if [ -f ${file} ]; then
        cp ${file} bin
    fi
done

tar czf ../cups.tar bin
