#!/bin/bash

function get_tsjdk_name()
{
    TSJDK8=tsjdk8
}

function change_file_to_unix()
{
    if [ -f $1 ]; then
        sed -i 's/\r//' $1
        return
    fi

    for item in `ls $1`
    do
        change_file_to_unix $1/$item
    done
}

function get_office_home_os()
{
    if [ "${1}" == "uos" ]; then
        TS_OFFICE_HOME=/opt/apps/cn.ts-it.office/files
    else
        TS_OFFICE_HOME=/opt/Taishan/Office
    fi
}

function get_office_home()
{
    get_os_name
    get_office_home_os ${OS_NAME}
}

function get_reader_home_os()
{
    if [ "${1}" == "uos" ]; then
        TS_READER_HOME=/opt/apps/cn.ts-it.reader/files
    else
        TS_READER_HOME=/opt/Taishan/Reader
    fi
}

function get_reader_home()
{
    get_os_name
    get_reader_home_os ${OS_NAME}
}

function get_gcc_params()
{
    GCC_VERSION=`g++ -v 2>&1>/dev/null | grep "gcc 版本 " | awk '{ print $3 }'`
    if [ "${GCC_VERSION}" == "" ]; then
        GCC_VERSION=`g++ -v 2>&1>/dev/null | grep "gcc version " | awk '{ print $3 }'`
    fi
    GCC_VERSION=`echo ${GCC_VERSION} | awk -F "." '{ print $1 }'`

    GCC_PARAMS="-fPIC"
    if [ "${GCC_VERSION}" == "5" ]; then
        GCC_PARAMS=${GCC_PARAMS}" -fno-pie -no-pie"
    fi
    echo ${GCC_PARAMS}
}

function change_file_to_unix()
{
    if [ -f $1 ]; then
        sed -i 's/\r//' $1
        return
    fi

    for item in `ls $1`
    do
        change_file_to_unix $1/$item
    done
}

function get_tsjdk_src()
{
    get_tsjdk_name
    get_cpu_info
    TSJDK_SRC=ts-jdk8
    if [ -d cpu-${CPU_JDK_SRC} ]; then
        cp -rf cpu-${CPU_JDK_SRC}/* ${TSJDK_SRC}
    fi
    echo get_tsjdk_src=${TSJDK_SRC}
}

function get_tsjdk_build_version()
{
    # shoud be tsjdk8 path
    TSJDK_VERSION_FILE=hotspot/make/hotspot_version
    if [ ! -f ${TSJDK_VERSION_FILE} ]; then
        get_tsjdk_src
        TSJDK_VERSION_FILE=${TSJDK_SRC}/${TSJDK_VERSION_FILE}
    fi
    
    if [ "${BUILD_VERSION}" == "" ]; then
        HS_MINOR_VER=HS_MINOR_VER
        BUILD_VERSION=`grep -rn "${HS_MINOR_VER}"   ${TSJDK_VERSION_FILE} | awk -F '=' '{print $2}'`
    fi

    if [ "${BUILD_NUMBER}" == "" ]; then
        HS_BUILD_NUMBER=HS_BUILD_NUMBER
        BUILD_NUMBER=`grep -rn "${HS_BUILD_NUMBER}" ${TSJDK_VERSION_FILE} | awk -F '=' '{print $2}'`
    fi    
}

function get_tsfreetype_path()
{
    get_lib_suffix
    LIBTSFREETYPE=libtsfreetype
    LIBTSFREETYPE_SO=${LIBTSFREETYPE}.${LIB_SUFFIX}
    if [ $OS_NAME == macosx ]; then
        LIBTSFREETYPE_SO_FULL=${LIBTSFREETYPE}.6.${LIB_SUFFIX}
    else
        LIBTSFREETYPE_SO_FULL=${LIBTSFREETYPE_SO}.6.18.3
    fi

    TSFREETYPE_PATH=`find . -name ts-freetype2`
    TSFREETYPE_PATH=`pwd`/${TSFREETYPE_PATH}
    echo ${TSFREETYPE_PATH}
}

function get_cups_dir()
{
    CUPS_DIR=cups-2.4.1
}

function get_cups_full_dir()
{
    get_cups_dir
    CUPS_FULL_DIR=`pwd`/${CUPS_DIR}
    echo ${CUPS_FULL_DIR}
}

function get_vs_name_windows()
{
    VS_INSTALL_DIR_2015="/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 14.0"
    if [ -d "${VS_INSTALL_DIR_2015}" ]; then
        VS_NAME_WINDOWS="Microsoft Visual Studio 14.0"
    else
        VS_NAME_WINDOWS="Microsoft Visual Studio 11.0"
    fi
}

function get_cpu_linux_gnu_path()
{
    get_cpu_info
    CPU_LINUX_GNU_PATH=/usr/lib/${CPU_INCLUDE}-linux-gnu

    if [ ! -d ${CPU_LINUX_GNU_PATH} ]; then
        CPU_LINUX_GNU_PATH=/usr/lib/${CPU_INCLUDE}-linux-gnuabi64
    fi

    if [ ! -d ${CPU_LINUX_GNU_PATH} ]; then
        CPU_LINUX_GNU_PATH=/usr/lib64
    fi

    if [ ! -d ${CPU_LINUX_GNU_PATH} ]; then
        CPU_LINUX_GNU_PATH=/usr/lib
    fi
}

function parse_tsjdk8_args()
{
    while [ $# -gt 0 ]
    do
         item=`echo $1 | awk -F '=' '{print $1}'`
        value=`echo $1 | awk -F '=' '{print $2}'`
        if [ "$item" == "build" ]; then
            BUILD_NUMBER=$value
        elif [ "$item" == "number" ]; then
            BUILD_NUMBER=$value
        elif [ "$item" == "debug" ]; then
            BUILD_DEBUG=$value
            if [ "${BUILD_DEBUG}" = "debug" ]; then
                BUILD_DEBUG=fastdebug
            else
                BUILD_DEBUG=release
            fi
        elif [ "$item" == "clean" ]; then
            BUILD_CLEAN=$value
        elif [ "$item" == "clear" ]; then
            BUILD_CLEAN=$value
        elif [ "$item" == "tsfreetype" ]; then
            BUILD_TSFREETYPE=$value
        elif [ "$item" == "cups" ]; then
            BUILD_CUPS=$value
        elif [ "$item" == "jdk" ]; then
            BUILD_JDK=$value
        elif [ "$item" == "tar" ]; then
            BUILD_TAR=$value
        elif [ "$item" == "bits" ]; then
            BUILD_BITS=$value
        elif [ "$item" == "os" ]; then
            BUILD_OS=$value
        elif [ "$item" == "cpu" ]; then
            BUILD_CPU=$value
        fi
        shift
    done

    if [ "${BUILD_BITS}" == "" ]; then
        BUILD_BITS=64
    fi

    if [ "${BUILD_DEBUG}" == "" ]; then
        BUILD_DEBUG=release
    fi
    
    if [ "${BUILD_OS}" == "" ]; then
        get_os_name
        BUILD_OS=${OS_NAME}
    fi

    if [ "${BUILD_CPU}" == "" ]; then
        get_cpu_info
        BUILD_CPU=${CPU_INSTALL_BUILD}
    fi

    if [ "${BUILD_TSFREETYPE}" == "" ]; then
        get_tsfreetype_path
        BUILD_TSFREETYPE=${TSFREETYPE_PATH}
    fi

    if [ "${BUILD_CUPS}" == "" ]; then
        get_cups_full_dir
        BUILD_CUPS=${CUPS_FULL_DIR}
    fi

    echo ${BUILD_CPU} ${BUILD_BITS} ${BUILD_NUMBER} ${BUILD_DEBUG} ${BUILD_CLEAN} ${BUILD_TSFREETYPE} ${BUILD_CUPS}
}

function get_system_jdk()
{
    JDK_ROOT=`find /usr/lib/jvm -name java*openjdk* | grep ${CPU_JDK_USR} `
    for jdk in ${JDK_ROOT}
    do
        JDK_ROOT=${jdk}
        break
    done
}

function get_build_jdk()
{
    get_os_name
    get_tsjdk_name
    if [ "${OS_NAME}" == "windows" ]; then
        BUILD_JDK=/cygdrive/d/${TSJDK8}-windows/${TSJDK8}-bin-64
        return
    fi
    
    JDK_FINDS=`ls -d ${TSJDK8}-${OS_NAME}* `
    for jdk in ${JDK_FINDS}
    do
        BUILD_JDK=${jdk}
        return
    done
    
    JDK_FINDS=`ls -d ~/${TSJDK8}-${OS_NAME}* `
    for jdk in ${JDK_FINDS}
    do
        BUILD_JDK=${jdk}
        return
    done
        
    JDK_FINDS=`ls -d /usr/lib/jvm/java-8-openjdk-* `
    for jdk in ${JDK_FINDS}
    do
        BUILD_JDK=${jdk}
        return
    done

    get_system_jdk
    BUILD_JDK=${JDK_ROOT}
}

function build_make_check_build_dir()
{
    BUILD_DIR=build

    if [ ! -d ${BUILD_DIR} ] ; then
        BUILD_CLEAN=true
    fi

    rm_file a.out
    rm_file config.log

    if [ "${BUILD_CLEAN}" = "true" ]; then
        DEL_FILES=".hg .jcheck"
        for file in ${DEL_FILES}
        do
            echo ${file}
            find -name ${file} | xargs rm -rf
        done

        if [ -d build ]; then
            echo rm -rf ${BUILD_DIR}
            rm -rf ${BUILD_DIR}
        fi
    fi
    MAKE_TARGETS=images    
}

function build_make_check_configure_action()
{
    if [ ${FLAG_CONFIGURE} != 1 ]; then
        return
    fi

    CONF_ARGS=" "
    CONF_ARGS=${CONF_ARGS}" "--with-milestone=taishan
    CONF_ARGS=${CONF_ARGS}" "--with-vendor-name=TSIT

    # 注意字串中日期不能有:
    RELEASE_SUFFIX=${OS_NAME}-${CPU_INSTALL_BUILD}-`date '+%Y_%m_%d_%H_%M_%S'`
    CONF_ARGS=${CONF_ARGS}" "--with-user-release-suffix=${RELEASE_SUFFIX}
    
    CONF_ARGS=${CONF_ARGS}" "--with-freetype=${BUILD_TSFREETYPE}
    CONF_ARGS=${CONF_ARGS}" "--with-target-bits=${BUILD_BITS}
    CONF_ARGS=${CONF_ARGS}" "--with-boot-jdk="${BUILD_JDK}"

    if [ "${BUILD_VERSION}" != "" ]; then
        CONF_ARGS=${CONF_ARGS}" "--with-update-version=${BUILD_VERSION}
    fi
    if [ "${BUILD_NUMBER}" != "" ]; then
        CONF_ARGS=${CONF_ARGS}" "--with-build-number=${BUILD_NUMBER}
    fi
    if [ "${BUILD_DEBUG}" != "" ]; then
        CONF_ARGS=${CONF_ARGS}" "--with-debug-level=${BUILD_DEBUG}
    fi

    echo ${CONF_ARGS}    
    echo run configure

    chmod 777 configure
    
    if [ "${OS_NAME}" == "${OS_WINDOWS}" ]; then
        # this has a space, 
        C_COMPILER="/cygdrive/c/Program Files (x86)/"${VS_NAME_WINDOWS}"/VC/bin"
        if [ ${BUILD_BITS} == 64 ] ; then
            C_COMPILER=${C_COMPILER}/amd64
        fi
        

        # C_COMPILER 有空格，所以只能在这里处理。
        ./configure ${CONF_ARGS} \
            --with-tools-dir="${C_COMPILER}" \
            --disable-ccache
    else
        ./configure ${CONF_ARGS}
    fi
}

function build_make_make_action()
{
    echo make ${MAKE_TARGETS}
    make JOBS=${CPU_CORE_AMOUNT} ${MAKE_TARGETS}
}

function mac_check_environments()
{
    if [ "${OS_NAME}" != "${OS_WINDOWS}" ]; then
        find -name .svn | xargs rm -rf
    fi

    local XCODE_TO_DEVELOPER_DIR=/Xcode.app/Contents/Developer
    export COMPILER_WARNINGS_FATAL=false
    export PATH=${HOME}${XCODE_TO_DEVELOPER_DIR}/usr/bin:${PATH}

    local APPS_TO_DEVELOPER_DIR=/Applications${XCODE_TO_DEVELOPER_DIR}

    local NEW_INCLUDE=${APPS_TO_DEVELOPER_DIR}/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
    export   CFLAGS="-I$NEW_INCLUDE"
    export CXXFLAGS="-I$NEW_INCLUDE"

    local NEW_LIB=${APPS_TO_DEVELOPER_DIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib
    export LDFLAGS="-L${NEW_LIB} -lstdc++ "
    export LIBRARY_PATH=${NEW_LIB}:${LIBRARY_PATH}
}

function mac_check_xcode()
{
    XCODE_LIBSTDC=xcode-missing-libstdc-
    if [ ! -f ${XCODE_LIBSTDC} ]; then
        tar xf dependence/${XCODE_LIBSTDC}.tar
    fi

    cd ${XCODE_LIBSTDC}
    ./install.sh
    cd ..
}