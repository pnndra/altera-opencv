DESCRIPTION = "ARM DS5 Gator Debugger"
AUTHOR = "Jim Rucker <jrucker@altera.com>"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM="file://driver/LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"
S="${WORKDIR}/git"
BP="${BPN}"
DEPENDS = "virtual/kernel"
inherit module

SRC_URI = "git://git.linaro.org/git-ro/arm/ds5/gator.git;protocol=http"
SRCREV="06ebd1eab0a782377611efee820bb57f09692cbf"
INHIBIT_PACKAGE_DEBUG_SPLIT_${PN} = "1"
INSANE_SKIP_${PN} += "ldflags"

do_compile() {
	echo "Using Custom compile step"
	cd ${S}/driver
	${MAKE} -C ${STAGING_KERNEL_DIR} M=`pwd` ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_PREFIX} modules
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	cd ${S}/daemon
	${MAKE} -j5 ARCH="arm" CROSS_COMPILE=${TARGET_PREFIX}
}

do_install() {
	LIB_DIR=${D}/${libdir}/modules
	install -d ${LIB_DIR}
	install -m 0644 ${S}/driver/gator.ko ${LIB_DIR}

	INIT_DIR=${D}${sysconfdir}/init.d/
	install -d ${INIT_DIR}
        install -m 0755 ${S}/daemon/gatord ${INIT_DIR}/gatord
}

FILES_${PN} = "${libdir}/modules/gator.ko ${sysconfdir}/init.d/gatord"
