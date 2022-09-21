#!/bin/bash

# lib_platform.sh

# UNION: same for different products, same for different versions.

# app: os think we install a application. when uninstall it we should use.
#      like: ts-office/ts-reader, cn.ts-it.office/cn.ts-it.reader

# product: we call our work.
#      like: Office/Reader

# TS_OFFICE_NAME: maybe Office, or ts-office. so don't use it.
#      You can use PRODUCT_OFFICE_NAME/APP_OFFICE_NAME/APP_NAME

function to_lowercase()
{
    LOWER_CASE=$(echo ${1} | tr '[A-Z]' '[a-z]')
    echo ${LOWER_CASE}
}

function to_uppercase()
{
    UPPER_CASE=$(echo ${1} | tr '[a-z]' '[A-Z]')
    echo ${UPPER_CASE}
}

# {{{{{{{{{{ os and cpu

function get_os_name()
{
    OS_WINDOWS="windows"
    # uos
    OS_UOS="uos"
    # Zhongbiao Kylin
    OS_NEOKYLIN="neokylin"
    # Yinhe Kylin
    OS_KYLIN="kylin"
    
    OS_MAC="macosx"
    
    if [ -d /cygdrive/d ]; then
        OS_NAME=${OS_WINDOWS}
        echo ${OS_NAME}
        return
    fi
    
    OS_RESULT=`uname`
    OS_RESULT=$(to_lowercase $OS_RESULT)

    if [ "${OS_RESULT}" == "darwin" ]; then
        OS_NAME=${OS_MAC}
        echo ${OS_NAME}
        return
    fi
    
    OS_RESULT=`cat  /etc/issue | awk '{print $1}' `
    OS_NAME=`echo ${OS_RESULT} | awk '{print $1}' `
    OS_NAME=$(to_lowercase $OS_NAME)
    if [ "${OS_NAME}" == "uniontech" ]; then
        OS_NAME=${OS_UOS}
    fi

    # echo ${OS_NAME}
}

function get_cpu_cores()
{
    if [ -f /proc/cpuinfo ]; then
        CPU_CORE_AMOUNT=`cat /proc/cpuinfo| grep "processor"| wc -l`
    else
        CPU_CORE_AMOUNT=`sysctl -n machdep.cpu.thread_count`
    fi
}

function get_cpu_info()
{
    get_os_name
    get_cpu_cores

    # x86/amd64
    CPU_NAME_INTEL="intel"
    CPU_NAME_AMD="amd"
    CPU_NAME_ZHAOXIN="zhaoxin"
    
    # arm64: CPU_PHYTIUM/CPU_FT is same manufactory
    CPU_NAME_PHYTIUM="phytium"
    CPU_NAME_FT="ft"

    # arm64
    CPU_NAME_KUNPENG="kunpeng"

    # mips64el
    CPU_NAME_LOONGSON="loongson"

    # loongarch64
    CPU_ARCH_LOONG="loongarch"

    # sw_64
    CPU_NAME_SUNWAY="sunway"

    if [ -f /proc/cpuinfo ]; then
        CPU_INFO=`cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c `
        if [[ "${CPU_INFO}" == "" ]]; then
            CPU_INFO=`cat /proc/cpuinfo | grep model | cut -f2 -d: | uniq -c `
        fi
    else
        CPU_INFO=`sysctl machdep.cpu.brand_string`
    fi
    CPU_INFO=$(to_lowercase "$CPU_INFO")

    if [[ ${CPU_INFO} == *${CPU_NAME_LOONGSON}* ]]; then
        CPU_NAME=loongson
        CPU_INFO=`cat /proc/cpuinfo | grep isa | cut -f2 -d: | uniq -c `
        if [[ ${CPU_INFO} == *${CPU_ARCH_LOONG}* ]]; then
            CPU_DEB=loongarch64
            CPU_JDK_USR=loongarch64
            CPU_JDK_LIB=loongarch64
            CPU_JDK_BUILD=loongarch64
            CPU_INSTALL_BUILD=loongarch64
            CPU_INCLUDE=loongarch64
            CPU_RES=loongarch64
            CPU_JDK_SRC=loongarch64
            CPU=__LOONGARCH64__
        else 
            CPU_DEB=mips64el
            CPU_JDK_USR=mips64el
            CPU_JDK_LIB=mips64el
            CPU_JDK_BUILD=mips64
            CPU_INSTALL_BUILD=mips64el
            CPU_INCLUDE=mips64el
            CPU_RES=mips64el
            CPU_JDK_SRC=mips64el
            CPU=__MIPS64EL__
        fi
    elif [[ ${CPU_INFO} == *${CPU_NAME_PHYTIUM}* || ${CPU_INFO} == *${CPU_NAME_FT}* ]]; then
        CPU_NAME=phytium
        CPU_DEB=arm64
        CPU_JDK_USR=arm64
        CPU_JDK_LIB=aarch64
        CPU_JDK_BUILD=aarch64
        CPU_INSTALL_BUILD=arm64
        CPU_INCLUDE=aarch64
        CPU_RES=arm64
        CPU_JDK_SRC=arm64
        CPU=__AARCH64__
    elif [[ ${CPU_INFO} == *${CPU_NAME_KUNPENG}* ]]; then
        CPU_NAME=kunpeng
        CPU_DEB=arm64
        CPU_JDK_USR=arm64
        CPU_JDK_LIB=aarch64
        CPU_JDK_BUILD=aarch64
        CPU_INSTALL_BUILD=arm64
        CPU_INCLUDE=aarch64
        CPU_RES=arm64
        CPU_JDK_SRC=arm64
        CPU=__AARCH64__
    elif [[ ${CPU_INFO} == *${CPU_NAME_ZHAOXIN}* ]]; then
        CPU_NAME=zhaoxin
        CPU_DEB=amd64
        CPU_JDK_USR=amd64
        CPU_JDK_LIB=amd64
        CPU_JDK_BUILD=amd64
        CPU_INSTALL_BUILD=amd64
        CPU_INCLUDE=x86_64
        CPU_RES=amd64
        CPU_JDK_SRC=amd64
        CPU=__AMD64__
    elif [[ ${CPU_INFO} == *${CPU_NAME_SUNWAY}* ]]; then
        CPU_NAME=sunway
        CPU_DEB=sw_64
        CPU_JDK_USR=sw64
        CPU_JDK_LIB=sw64
        CPU_JDK_BUILD=sw64
        CPU_INSTALL_BUILD=sw_64
        CPU_INCLUDE=sw_64
        CPU_RES=sw_64
        CPU_JDK_SRC=sw_64
        CPU=__SW64__
    else
        CPU_NAME=intel
        CPU_DEB=amd64
        CPU_JDK_USR=amd64
        CPU_JDK_LIB=amd64
        CPU_JDK_BUILD=x86_64
        CPU_INSTALL_BUILD=amd64
        CPU_INCLUDE=x86_64
        CPU_RES=amd64
        CPU_JDK_SRC=amd64
        CPU=__AMD64__
    fi

    # CPU_INCLUDE=`uname -m`
    # CPU_INCLUDE=`/bin/arch`

    echo   ${CPU_NAME} ${CPU_JDK_USR} ${CPU_JDK_LIB} ${CPU_JDK_BUILD} ${CPU}
    # return ${CPU_NAME} ${CPU_JDK_LIB} ${CPU_JDK_BUILD}
}

# }}}}}}}}}} os and cpu

# {{{{{{{{{{ init means from nothing to create

function init_product_name_1param()
{
    PRODUCT_NAME=${1}
    PRODUCT_NAME_L=$(to_lowercase $PRODUCT_NAME)
}

function init_office_name()
{
    PRODUCT_OFFICE_NAME=Office
    init_product_name_1param ${PRODUCT_OFFICE_NAME}
}

function init_reader_name()
{
    PRODUCT_READER_NAME=Reader
    init_product_name_1param ${PRODUCT_READER_NAME}
}

# function init_product_name():
# You can find it in lib_product.sh/lib_product_office.sh/lib_product_reader.sh

# }}}}}}}}}} init means from nothing to create

# {{{{{{{{{{ get_app_name

function get_app_name_2param()
{
    init_product_name_1param ${1}
    
    if [ "${2}" == "${OS_UOS}" ]; then
        APP_NAME=cn.ts-it.${PRODUCT_NAME_L}
    else
        APP_NAME=ts-${PRODUCT_NAME_L}
    fi
}

function get_app_name_1param()
{
    get_os_name
    get_app_name_2param ${1} ${OS_NAME}
}

function get_office_name_1param()
{
    init_office_name
    get_app_name_2param ${PRODUCT_OFFICE_NAME} ${1}
    APP_OFFICE_NAME=${APP_NAME}
}

function get_office_name()
{
    get_os_name
    get_office_name_1param ${OS_NAME}
}

function get_reader_name_1param()
{
    init_reader_name
    get_app_name_2param ${PRODUCT_READER_NAME} ${1}
    APP_READER_NAME=${APP_NAME}
}

function get_reader_name()
{
    get_os_name
    get_reader_name_1param ${OS_NAME}
}

# }}}}}}}}}} get_app_name


# {{{{{{{{{{ get_app_home

function get_app_home_2param()
{
    init_product_name_1param ${1}

    if [ "${2}" == "${OS_UOS}" ]; then
        APP_HOME=/opt/apps/cn.ts-it.${PRODUCT_NAME_L}
        APP_FILES_DIR=${APP_HOME}/files
        APP_SYSTEM_DIR=${APP_HOME}/entries
    else
        APP_HOME=/opt/Taishan/${PRODUCT_NAME}
        APP_FILES_DIR=${APP_HOME}
        APP_SYSTEM_DIR=/
    fi
    
}

function get_office_home_1param()
{
    init_office_name
    get_app_home_2param ${PRODUCT_OFFICE_NAME} ${1}
    APP_OFFICE_HOME=${APP_HOME}
}

function get_office_home()
{
    get_os_name
    get_office_home_1param ${OS_NAME}
}

function get_reader_home_1param()
{
    init_reader_name
    get_app_home_2param ${PRODUCT_READER_NAME} ${1}
    APP_READER_HOME=${APP_HOME}
}

function get_reader_home()
{
    get_os_name
    get_reader_home_1param ${OS_NAME}
}

# }}}}}}}}}} get_app_home

function get_date_text()
{
    DATE_TEXT=`date +%Y-%m-%d`
    echo ${DATE_TEXT}
}

function get_time_text()
{
    TIME_TEXT=`date +%H:%M:%S`
    echo ${TIME_TEXT}
}

function get_lib_suffix()
{
    get_os_name
    if [ ${OS_NAME} == ${OS_WINDOWS} ]; then
        LIB_SUFFIX=dll
    elif [ ${OS_NAME} == ${OS_MAC} ]; then
        LIB_SUFFIX=dylib
    else
        LIB_SUFFIX=so
    fi
}

function copy_config_guess()
{
    for file in config.guess config.sub
    do
        cp_file /usr/share/misc/${file} ${1}
    done
}

# {{{{{{{{{{ some commands, we use it to avoid warning

function is_dir_has_files()
{
    if [ "`ls -A ${1}`" != "" ]; then
        echo true
    else
        echo false
    fi
}

function check_user_exist()
{
	typeset -i COUNT
	COUNT=`cat /etc/passwd | cut -f1 -d':' | grep -w "$1" -c`

	if [ $COUNT -le 0 ]; then
		echo "User $1 is not in /etc/passwd file"
		return 0
	else
		return 1
	fi	
}


function kill_process()
{
    #ps aux | grep -i $1 | grep -v grep | awk -e '{print $2}' | xargs -r sudo kill -9 1>/dev/null 2>&1
    SEARCH=`ps -A | grep $1`
    # echo ${SEARCH}
    if [ "${SEARCH}" == "" ]; then
        return
    fi
    killall $1
}

function link_src_to_dest()
{
    if [ ! -f $1 ]; then
        return
    fi
    if [ -L $2 -o -f $2 ]; then
        rm $2
    fi
    ln -snf $1 $2
}

function mk_dir()
{
    if [ ! -d $1 ]; then
        mkdir $1
    fi
}

function mk_dirs()
{
    if [ ! -d $1 ]; then
        mkdir -p $1
    fi
}

function rm_file()
{
    if [ -f "$1" -o -L "$1" ]; then
        rm "$1"
    fi
}

function rm_files()
{
    for file in "$@"
    do
        rm_file "$file"
    done
}

function rm_dir()
{
    if [ -d "$1" ]; then
        sudo rm -rf "$1"
    fi
}

function cp_file_to()
{
    if [ ! -f $1 ]; then
        return
    fi

    cp $1 $2
}

function cp_file_to_dir()
{
    if [ ! -f $1 ]; then
        return
    fi

    if [ ! -d $2 ]; then
        return
    fi
    
    cp $1 $2
}

function cp_dir_to()
{
    if [ ! -d $1 ]; then
        return
    fi

    rm_dir $2

    cp -rf $1 $2
}

# }}}}}}}}}} some commands, we use it to avoid warning

