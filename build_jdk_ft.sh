echo
echo {{{{{{{{{{ build_jdk_ft.sh {{{{{{{{{{
echo

BUILD_ARGS=${@:+"$@"}
. ./lib_platform.sh
. ./lib_tsjdk8.sh

get_tsfreetype_path

parse_tsjdk8_args ${BUILD_ARGS}

if [ "${BUILD_BITS}" = "" ] ; then
    BUILD_BITS=64
fi

if [ "${BUILD_TSFREETYPE}" = "" ] ; then
    BUILD_TSFREETYPE=${TSFREETYPE_PATH}
fi

if [ -d /cygdrive/d ]; then
    if [ ${BUILD_BITS} == 64 ] ; then
        TSFREETYPE_SRC=x64
    else
        TSFREETYPE_SRC=Win32
    fi
    TSFREETYPE_SRC=${BUILD_TSFREETYPE}/objs/${TSFREETYPE_SRC}/Release
    
    if [ ! -d ${TSFREETYPE_SRC} ]; then
        TSFREETYPE_SRC=libs/windows-${BUILD_BITS}
    fi
    
    TSFREETYPE_FILE=tsfreetype.dll
else
    TSFREETYPE_SRC=${BUILD_TSFREETYPE}/objs/.libs/
    if [ ! -d ${TSFREETYPE_SRC} ]; then
        TSFREETYPE_SRC=libs/${OS_NAME}-${BUILD_BITS}
    fi

    TSFREETYPE_FILE=${LIBTSFREETYPE_SO}
fi

echo ++++++++++ copy lib for tsfreetype ++++++++++
TSFREETYPE_LIB=${BUILD_TSFREETYPE}/lib
if [ -f ${TSFREETYPE_SRC}/${TSFREETYPE_FILE} ] ; then
    rm -rf ${TSFREETYPE_LIB}
    mkdir  ${TSFREETYPE_LIB}
    
    for file in ${TSFREETYPE_FILE} tsfreetype.lib tsfreetype.a
    do
        if [ -f ${TSFREETYPE_SRC}/${file} ]; then
            cp  ${TSFREETYPE_SRC}/${file} ${TSFREETYPE_LIB}
        fi
    done
fi

echo
echo }}}}}}}}}} build_jdk_ft.sh }}}}}}}}}}
echo
