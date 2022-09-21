BUILD_ARGS=${@:+"$@"}

. ./lib_platform.sh
. ./lib_tsjdk8.sh

get_os_name
get_cpu_info
get_tsjdk_src
get_tsfreetype_path

parse_tsjdk8_args ${BUILD_ARGS}
pwd

FLAG_2TAR=1
FLAG_JRE2OFFICE=0
FLAG_JDK2HOME=0

ROOT_DIR=`pwd`
TSJDK_DIR=${ROOT_DIR}/${TSJDK_SRC}

JDK=jdk
if [ "${BUILD_TAR}" = "jdk" ]; then
    SRC_IMAGE=j2sdk-image
    JRE_ROOT=/jre
    FLAG_JDK2HOME=1
else
    SRC_IMAGE=j2re-image
    JRE_ROOT=
    FLAG_JRE2OFFICE=1
fi

if [ ${FLAG_2TAR} == 1 ]; then
    TAR_FILE=ts${BUILD_TAR}8.tar
    if [ -f ${TAR_FILE} ]; then
        rm ${TAR_FILE}
    fi
fi

TSFREETYPE_DIR=${SRC_IMAGE}${JRE_ROOT}/lib
if [ ${OS_NAME} == ${OS_MAC} ]; then
    DIR_PREFIX=${OS_MAC}
else
    DIR_PREFIX=linux
    TSFREETYPE_DIR=${TSFREETYPE_DIR}/${CPU_JDK_LIB}
fi

BUILD_IMAGES_DIR=${TSJDK_DIR}/build/${DIR_PREFIX}-${CPU_JDK_BUILD}-normal-server-${BUILD_DEBUG}/images

cd ${BUILD_IMAGES_DIR}
if [ -d ${JDK}/src ]; then
    echo path error ! ${JDK}
    exit
fi

if [ -d ${JDK} ]; then
    rm -rf ${JDK}
fi

if [ -d ${SRC_IMAGE}/${JDK} ]; then
    rm -rf ${SRC_IMAGE}/${JDK}
fi

if [ ! -d ${SRC_IMAGE} ]; then
    echo not found ${SRC_IMAGE}
    exit
fi

if [ -d ${ROOT_DIR}/jarlib ]; then
     cp ${ROOT_DIR}/jarlib/* ${BUILD_IMAGES_DIR}/${SRC_IMAGE}/lib/ext
fi

# {{{{{{{{{{

cd ${TSFREETYPE_DIR}

LIBTSFREETYPE_SRC=${LIBTSFREETYPE_SO_FULL}
if [ ! -f ${BUILD_TSFREETYPE}/lib/${LIBTSFREETYPE_SRC} ]; then
    LIBTSFREETYPE_SRC=${LIBTSFREETYPE_SO}
fi
cp ${BUILD_TSFREETYPE}/lib/${LIBTSFREETYPE_SRC} ${LIBTSFREETYPE_SO_FULL}

if [ -f ${LIBTSFREETYPE_SO_FULL} ]; then
    ln -snf ${LIBTSFREETYPE_SO_FULL} ${LIBTSFREETYPE_SO}
    ln -snf ${LIBTSFREETYPE_SO_FULL} ${LIBTSFREETYPE_SO}.6
else
    rm ${LIBTSFREETYPE_SO}*
fi

cd ${BUILD_IMAGES_DIR}
pwd

# }}}}}}}}}}

# {{{{{{{{{{ copy CUPS file

if [ -d ${BUILD_CUPS} ]; then

    DEST_CUPS=${SRC_IMAGE}${JRE_ROOT}/bin/cups
    if [ ! -d ${DEST_CUPS} ]; then
        mkdir ${DEST_CUPS}
    fi

    for file in lpr lprm lpq lpc
    do
        file=${BUILD_CUPS}/berkeley/${file}
        if [ -f ${file} ]; then
            cp ${file} ${DEST_CUPS}
        fi
    done

fi

# }}}}}}}}}}

# {{{{{{{{{{ copy libpng.so.12.0

DEST_LIB=${SRC_IMAGE}${JRE_ROOT}/lib/${CPU_JDK_LIB}
cp ${ROOT_DIR}/libs/${OS_NAME}-${CPU_JDK_SRC}/* ${DEST_LIB}

# }}}}}}}}}}

# {{{{{{{{{{

mv ${SRC_IMAGE} ${JDK}

if [ ${FLAG_2TAR} == 1 ]; then
    echo tar file now
    find ${JDK} -name *.diz | xargs rm -rf
    tar czf ${TAR_FILE} ${JDK}
    mv ${TAR_FILE}  ${ROOT_DIR}
fi

mv ${JDK} ${SRC_IMAGE}

if [ ${FLAG_JDK2HOME} == 1 ]; then
    DEST_PATH=~/${TSJDK8}-${OS_NAME}-${BUILD_BITS}
    rm -rf ${DEST_PATH}
    cp -rf ${SRC_IMAGE} ${DEST_PATH}
fi

# }}}}}}}}}}
