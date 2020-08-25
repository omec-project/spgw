#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

CPUS=${CPUS:-'5'}
DPDK_VER=${DPDK_VER:-'18.02.2'}
export RTE_SDK=${RTE_SDK:-$DEPS_DIR/dpdk}
export RTE_TARGET=${RTE_TARGET:-'x86_64-native-linuxapp-gcc'}
export RTE_MACHINE=${RTE_MACHINE:-'native'}

install_dpdk() 
{

	[ -d $RTE_SDK ] && echo "DPDK already exists at $RTE_SDK" && return

	echo "Current direcrtory $PWD"
	echo "RTE_SDK = $RTE_SDK "
	mkdir -p ${RTE_SDK} && cd ${RTE_SDK}/../
	wget http://fast.dpdk.org/rel/dpdk-${DPDK_VER}.tar.xz
	tar -xvf dpdk-${DPDK_VER}.tar.xz -C ${RTE_SDK} --strip-components 1

	echo "Applying AVX not supported patch for resolved dpdk-18.02 i40e driver issue.."
	patch -d ${RTE_SDK} -p1 < $RTE_SDK/../../patches/v2-net-i40e-fix-avx2-driver-check-for-rx-rearm.diff
	if [ $? -ne 0 ] ; then
		echo "Failed to apply AVX patch, please check the errors."
		return
	fi

	cd ${RTE_SDK}
	sed -ri 's,(IGB_UIO=).*,\1n,' config/common_linuxapp
	sed -ri 's,(KNI_KMOD=).*,\1n,' config/common_linuxapp
	make --trace --debug -j $CPUS install T=${RTE_TARGET} RTE_MACHINE=${RTE_MACHINE} 
	echo "Installed DPDK at $RTE_SDK"
}

(return 2>/dev/null) && echo "Sourced" && return
