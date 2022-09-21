#!/bin/bash

function build_make_check_build_version()
{
    VERSION_FILE=hotspot/make/hotspot_version
    if [ "${BUILD_NUMBER}" == "" ]; then
        get_tsjdk_build_version
    else
        HS_MINOR_VER=HS_MINOR_VER
        sed -i "/^${HS_MINOR_VER}=/c${HS_MINOR_VER}=${BUILD_VERSION}"      ${VERSION_FILE}

        HS_BUILD_NUMBER=HS_BUILD_NUMBER
        sed -i "/^${HS_BUILD_NUMBER}=/c${HS_BUILD_NUMBER}=${BUILD_NUMBER}" ${VERSION_FILE}

        # cat ${VERSION_FILE}
    fi
}

function build_make_check_windows_bits()
{
    # windows diff 32/64
    if [ "${OS_NAME}" == "${OS_WINDOWS}" ]; then
        if [ ${BUILD_BITS} = 32 ]; then
            CPU_INSTALL_BUILD=x86
        fi
    
        BUILD_DIR_BACKUP=${BUILD_DIR}-${BUILD_BITS}
        if [ -d ${BUILD_DIR_BACKUP} ] ; then
            echo use last time compiled result
            mv ${BUILD_DIR_BACKUP} ${BUILD_DIR}
        fi
    fi
}

BUILD_ARGS=${@:+"$@"}

. ../lib_platform.sh
. ../lib_tsjdk8.sh

get_os_name
get_cpu_info
get_vs_name_windows

parse_tsjdk8_args ${BUILD_ARGS}

if [ "${BUILD_JDK}" = "" ]; then
    get_build_jdk
fi

FLAG_CONFIGURE=1

if [ ${OS_NAME} == macosx ]; then
    mac_check_environments
fi


build_make_check_build_version

build_make_check_windows_bits

build_make_check_build_dir

build_make_check_configure_action

build_make_make_action
