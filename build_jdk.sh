#!/bin/bash

function build_jdk_copy_action()
{
    echo tar files

    if [ ${OS_NAME} == ${OS_WINDOWS} ]; then
        if [ ${BUILD_BITS} = 64 ]; then
            BITS_SUFFIX=_${BUILD_BITS}
        fi
        ./build_jdk_cp_windows.sh ${ARG_TSFREETYPE} ${ARG_CUPS} ${BUILD_ARGS} ${ARG_JDK}
    else
        ./build_jdk_cp_linux.sh   ${ARG_TSFREETYPE} ${ARG_CUPS} ${BUILD_ARGS} ${ARG_JDK} tar=jre
        ./build_jdk_cp_linux.sh   ${ARG_TSFREETYPE} ${ARG_CUPS} ${BUILD_ARGS} ${ARG_JDK} tar=jdk
    fi
}

function build_jdk_make_action()
{
    cp build_jdk_make.sh ${TSJDK_SRC}

    cd ${TSJDK_SRC}
    echo make now!
    ./build_jdk_make.sh ${ARG_TSFREETYPE} ${ARG_CUPS} ${ARG_BITS} ${BUILD_ARGS} ${ARG_JDK}

    cd ..
}

function build_jdk_create_args()
{
    ARG_BITS=bits=${BUILD_BITS}
    ARG_TSFREETYPE=tsfreetype=${BUILD_TSFREETYPE}
    ARG_CUPS=cups=${CUPS_FULL_DIR}
    echo ${TSJDK_SRC}

    if [ "${OS_NAME}" == "windows" ]; then
        ARG_JDK=jdk=`pwd`/${TSJDK8}-bin-${BUILD_BITS}
    fi
}

function build_jdk_copy_taishan_files()
{
    # copy cpu files to tsjdk8
    if [[ ${CPU_JDK_SRC} == "mips64el" || ${CPU_JDK_SRC} == loongarch64 ]]; then
        local LONGXIN_DIR=cpu-mips64el-loongarch64
        cp -rf ${LONGXIN_DIR}-simple/*  ${TSJDK_SRC}
        cp -rf ${LONGXIN_DIR}-complex/* ${TSJDK_SRC}
    fi

    # already same.
    # cp -rf ts-work/* ${TSJDK_SRC}
}

# number=09 debug=release clean=true
#  build=09 debug=release clean=true

BUILD_ARGS=${@:+"$@"}

. ./lib_platform.sh
. ./lib_tsjdk8.sh

get_os_name
get_cpu_info
get_tsjdk_src
get_tsfreetype_path
get_cups_full_dir
parse_tsjdk8_args ${BUILD_ARGS}

build_jdk_create_args

./build_jdk_ft.sh   ${ARG_TSFREETYPE} ${ARG_CUPS} ${ARG_BITS} ${BUILD_ARGS} ${ARG_JDK}

build_jdk_copy_taishan_files

build_jdk_make_action

build_jdk_copy_action

