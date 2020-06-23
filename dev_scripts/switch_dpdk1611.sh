#! /bin/bash

# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

S1UDEV=0000:af:00.0
SGIDEV=0000:af:00.1

echo -e "Switching to DPDK 16.11...\n"
echo "Path to dpdk16.11 @/home/intel-lab/ngic-rtc-dbg/dpdk-1611..."
source setenv_dpdk1611.sh
echo -e "insmod DPDK 16.11 igb_uio @$RTE_SDK/$RTE_TARGET...\n"
rmmod igb_uio
insmod $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko
lsmod | grep uio*
echo -e "...\n"
pushd dp
echo  "Building dp..."
make clean; make
echo -e "\nBind $S1UDEV & $SGIDEV to DPDK 16.11 igb_uio..."
$RTE_SDK/tools/dpdk-devbind.py -b igb_uio $S1UDEV $SGIDEV
$RTE_SDK/tools/dpdk-devbind.py -s | grep '$S1UDEV\|$S1UDEV'
popd
echo -e "...\n"
pushd cp
echo  "Building cp..."
make clean; make
echo -e "\n**** System switched to dpdk-1611. RTE_SDK=$RTE_SDK ****\n"

