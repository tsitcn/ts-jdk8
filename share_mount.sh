SHARE_DIR=WINDOWS_D
if [ ! -d ${SHARE_DIR} ]; then
    mkdir ${SHARE_DIR}
fi

sudo mount -t vboxsf D_DRIVE ~/${SHARE_DIR}

