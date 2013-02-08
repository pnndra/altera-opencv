inherit kernel
SECTION = "kernel"
DESCRIPTION = "Altera Linux kernel"
LICENSE = "GPLv2"

COMPATIBLE_MACHINE = "socfpga"

KBRANCH = "${MACHINE}-${LINUX_VERSION}"

do_configure() {
	export INITRAMFS_IMAGE_FILE="${INITRAMFS_IMAGE}-${MACHINE}.cpio"

	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	export CROSS_COMPILE="${TARGET_PREFIX}"
	export ARCH=${ARCH}
	
	oe_runmake ${MACHINE_DEFCONFIG} CONFIG_BLK_DEV_INITRD=y
	
	cp .config .config.ori

	grep CONFIG_BLK_DEV_INITRD .config || echo "CONFIG_BLK_DEV_INITRD=y" >> .config
	grep CONFIG_INITRAMFS_SOURCE .config || echo "CONFIG_INITRAMFS_SOURCE=\"\"" >> .config
	grep CONFIG_INITRAMFS_ROOT_UID .config || echo "CONFIG_INITRAMFS_ROOT_UID=0" >> .config
	grep CONFIG_INITRAMFS_ROOT_GID .config || echo "CONFIG_INITRAMFS_ROOT_GID=0" >> .config
	grep CONFIG_INITRAMFS_COMPRESSION_NONE .config || echo "CONFIG_INITRAMFS_COMPRESSION_NONE=y" >> .config

	if [ ! -z ${INITRAMFS_IMAGE} ] && [ -e ${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_FILE} ]; then
		cp ${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE_FILE} .
		echo "Found valid initramfs ${INITRAMFS_IMAGE_FILE}"
		sed -i "s|^.*CONFIG_BLK_DEV_INITRD[ =].*$|CONFIG_BLK_DEV_INITRD=y|g" .config
		sed -i "s|^.*CONFIG_INITRAMFS_SOURCE[ =].*$|CONFIG_INITRAMFS_SOURCE=\"${INITRAMFS_IMAGE_FILE}\"|g" .config
		sed -i "s|^.*CONFIG_INITRAMFS_ROOT_UID[ =].*$|CONFIG_INITRAMFS_ROOT_UID=0|g" .config
		sed -i "s|^.*CONFIG_INITRAMFS_ROOT_GID[ =].*$|CONFIG_INITRAMFS_ROOT_GID=0|g" .config
		sed -i "s|^.*CONFIG_INITRAMFS_COMPRESSION_NONE[ =].*$|CONFIG_INITRAMFS_COMPRESSION_NONE=y|g" .config
		oe_runmake oldconfig
	else
		rm -f *.cpio
		echo "Initramfs disabled or can't find valid initramfs ${INITRAMFS_IMAGE_FILE}"
		sed -i "s|^CONFIG_BLK_DEV_INITRD=.*$|# CONFIG_BLK_DEV_INITRD is not set|g" .config
		sed -i "s|^CONFIG_INITRAMFS_SOURCE=\".*\"$|# CONFIG_INITRAMFS_SOURCE is not set|g" .config
		sed -i "s|^CONFIG_INITRAMFS_ROOT_UID=.*$|# CONFIG_INITRAMFS_ROOT_UID is not set|g" .config
		sed -i "s|^CONFIG_INITRAMFS_ROOT_GID=.*$|# CONFIG_INITRAMFS_ROOT_GID is not set|g" .config
		sed -i "s|^CONFIG_INITRAMFS_COMPRESSION_NONE=.*$|# CONFIG_INITRAMFS_COMPRESSION_NONE is not set|g" .config
		oe_runmake oldconfig
	fi	
}

kernel_do_compile() {
        unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
        oe_runmake __headers CC="${KERNEL_CC}" LD="${KERNEL_LD}"
        oe_runmake ${KERNEL_IMAGETYPE_FOR_MAKE} ${KERNEL_ALT_IMAGETYPE} CC="${KERNEL_CC}" LD="${KERNEL_LD}" LOADADDR=0x8000
        if test "${KERNEL_IMAGETYPE_FOR_MAKE}.gz" = "${KERNEL_IMAGETYPE}"; then
                gzip -9c < "${KERNEL_IMAGETYPE_FOR_MAKE}" > "${KERNEL_OUTPUT}"
        fi
}

do_compile_dtb() {
        unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
        export CROSS_COMPILE="${TARGET_PREFIX}"
        export ARCH=${ARCH}
        oe_runmake dtbs
}

addtask compile_dtb after do_compile before do_build

do_deploy_dtb() {
    install -d ${DEPLOY_DIR_IMAGE}
    if test -n "${KERNEL_DEVICETREE}"; then
        for DTS_FILE in ${KERNEL_DEVICETREE}; do
            if [ ! -f ${DTS_FILE} ]; then
                echo "Warning: ${DTS_FILE} is not available!"
                continue
            fi
	    DEVICE_TREE_OUTPUT="arch/${ARCH}/boot/${MACHINE}.dtb"
            DTS_BASE_NAME=`basename ${DTS_FILE} | awk -F "." '{print $1}'`
            DTB_NAME="${MACHINE}-${DATETIME}"
	    DTB_SYMLINK_NAME="${MACHINE}"
            install -m 0644 ${DEVICE_TREE_OUTPUT} ${DEPLOY_DIR_IMAGE}/${DTB_NAME}.dtb
            cd ${DEPLOY_DIR_IMAGE}
            rm -f ${DTB_SYMLINK_NAME}.bin
            ln -sf ${DTB_NAME}.dtb ${DTB_SYMLINK_NAME}.dtb
            cd -
       done
    fi
}

addtask deploy_dtb after do_compile_dtb before do_build

VMLINUX_NAME ?= "vmlinux-${PV}-${PR}-${MACHINE}-${DATETIME}"

kernel_do_deploy_append() {
	install -m 0644 ${S}/vmlinux ${DEPLOYDIR}/${VMLINUX_NAME}
	rm -f vmlinux
	ln -sf ${VMLINUX_NAME} vmlinux
}
