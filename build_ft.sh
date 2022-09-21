#!/bin/bash

function build_ft_clean()
{
    cd ${TSFREETYPE_PATH}
    find -name .svn | xargs rm -rf

    # rm_file builds/unix/libtool
    rm_file builds/unix/config.status

    make clean

    for item in objs lib
    do
        if [ ! -d ${item} ]; then
            mk_dir ${item}
        else
            rm -rf ${item}/*
        fi
    done
}

function build_ft_copy_4_linux()
{
    cp ts-linux/* include/freetype
}

function build_ft_make()
{
    for item in include src
    do
        change_file_to_unix $item
    done

    chmod 777 builds/unix/configure
    chmod 777 configure
    ./configure \
        --with-png=no

    make -j${CPU_CORE_AMOUNT}
}

function build_ft_copy_so()
{
    cp_file_to_dir   ${LIBTSFREETYPE_SO_SRC} lib

    cd lib
    link_src_to_dest ${LIBTSFREETYPE_SO_FULL} ${LIBTSFREETYPE_SO}
    link_src_to_dest ${LIBTSFREETYPE_SO_FULL} ${LIBTSFREETYPE_SO}.6

    # for tsjdk build
    # cp ${LIBTSFREETYPE_SO} libtsfreetype.${LIB_SUFFIX}
}

. ./lib_platform.sh
. ./lib_tsjdk8.sh

get_cpu_cores
get_tsfreetype_path

build_ft_clean

build_ft_copy_4_linux

build_ft_make

LIBTSFREETYPE_SO_SRC=objs/.libs/${LIBTSFREETYPE_SO_FULL}
if [ ! -f ${LIBTSFREETYPE_SO_SRC} ]; then
    echo !!!! NO FILE ${LIBTSFREETYPE_SO_SRC}
    exit
fi

build_ft_copy_so
