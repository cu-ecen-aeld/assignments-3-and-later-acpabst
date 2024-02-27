#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    echo "Kernel Build"
    sudo apt update
    sudo apt-get install -y --no-install-recommends bc u-boot-tools kmod cpio flex bison libssl-dev psmisc libncurses-dev libelf-dev
    sudo apt-get install -y qemu-system-arm
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    echo "Adding the Image in outdir"
    ln -s ${OUTDIR}/linux-stable/arch/arm64/boot/Image ${OUTDIR}/Image
fi

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi
mkdir -p rootfs
cd rootfs
mkdir -p bin dev etc home lib lib64 proc root sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
cd ..

echo "Make and install busybox"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    make distclean
    make defconfig
else
    cd busybox
fi
make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} 
make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX=${OUTDIR}/rootfs install
cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

echo `${CROSS_COMPILE}gcc -print-sysroot`
CROSS_COMPILE_PATH=$(${CROSS_COMPILE}gcc -print-sysroot)
sudo cp ${CROSS_COMPILE_PATH}/lib/ld-linux-aarch64.so.1 lib/ld-linux-aarch64.so.1

sudo cp ${CROSS_COMPILE_PATH}/lib64/libm.so.6 lib64/libm.so.6
sudo cp ${CROSS_COMPILE_PATH}/lib64/libresolv.so.2 lib64/libresolv.so.2
sudo cp ${CROSS_COMPILE_PATH}/lib64/libc.so.6 lib64/libc.so.6

echo "Make device nodes"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

echo "Clean and build the writer utility"
cd ${FINDER_APP_DIR}

echo "Copy the finder related scripts and executables over"
make writer CROSS_COMPILE=gcc
cp writer ${OUTDIR}/rootfs/home/writer

cp finder.sh ${OUTDIR}/rootfs/home/finder.sh
mkdir ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf/username.txt
cp conf/assignment.txt ${OUTDIR}/rootfs/home/conf/assignment.txt
cp finder-test.sh ${OUTDIR}/rootfs/home/finder-test.sh

sed -n 's+../conf/assignment.txt+/conf/assignment.txt+g' ${OUTDIR}/rootfs/home/finder-test.sh

cp autorun-qemu.sh ${OUTDIR}/rootfs/home/autorun-qemu.sh

echo "Chown the root directory"
sudo chown root:root *

echo "Create initramfs.cpio.gz"
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ..
gzip -f initramfs.cpio
sudo chown root:root initramfs.cpio.gz
