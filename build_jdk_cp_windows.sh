BUILD_ARGS=${@:+"$@"}

. ./lib_platform.sh
. ./lib_tsjdk8.sh

get_tsjdk_src
parse_tsjdk8_args ${BUILD_ARGS}

echo
echo {{{{{{{{{{ build_jdk_cp_windows.sh {{{{{{{{{{
echo

BUILD_RESULT=windows-x86
if [ ${BUILD_BITS} = 64 ]; then
    BUILD_RESULT=${BUILD_RESULT}_64
fi
BUILD_RESULT=${BUILD_RESULT}-normal-server-release

IMAGE_DIR=${TSJDK_SRC}/build/${BUILD_RESULT}/images
# diz file?
find ${IMAGE_DIR} -name *.diz | xargs rm -rf

PROJECT_DIR=Z:/Office-3.5-project
for dir in Office-3.5 Resources/TsJDK8-bin/Windows
do
    DEST_DIR=${PROJECT_DIR}/${dir}/tsjdk8-${BUILD_BITS}
    rm -rf ${DEST_DIR}
    if [ -d ${DEST_DIR} ]; then
        DEST_DIR=${DEST_DIR}/*
    fi
    cp -rf ${IMAGE_DIR}/j2sdk-image ${DEST_DIR}
done

echo
echo }}}}}}}}}} build_jdk_cp_windows.sh }}}}}}}}}}
echo
