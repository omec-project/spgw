#! /bin/bash

# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

S1UDEV=0000:af:00.0
SGIDEV=0000:af:00.1

echo "Switching to ngic4fx baseline..."
pushd dp
cp Makefile_wkni Makefile
cp ether_Oct18.c ether.c
popd
pushd interface
cp interface_Oct18.c interface.c
popd
echo -e "Replaced ~/dp/Makefile, ~/dp/ether.c. ~/interface/interface.c files\n"

echo "Path to dpdk18.02 @/home/intel-lab/ngic-rtc-dbg/dpdk-1802..."
source setenv_dpdk1802.sh
echo -e "insmod DPDK 18.02 igb_uio @$RTE_SDK/$RTE_TARGET...\n"
rmmod igb_uio
insmod $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko
lsmod | grep uio*
echo -e "...\n"
pushd dp
echo  "Building dp..."
make clean; make
echo -e "\nBind $S1UDEV & $SGIDEV to DPDK 18.02 igb_uio..."
$RTE_SDK/usertools/dpdk-devbind.py -b igb_uio $S1UDEV $SGIDEV
$RTE_SDK/usertools/dpdk-devbind.py -s | grep '$S1UDEV\|$S1UDEV'
popd
echo -e "...\n"
pushd cp
echo  "Building cp..."
make clean; make
echo -e "\n**** System switched to Oct18 base on dpdk-1802. RTE_SDK=$RTE_SDK ****\n"

