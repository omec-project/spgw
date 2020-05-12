#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

source ../git_url.cfg
export NGIC_DIR=$PWD

SERVICE_NAME="CP"
SERVICE=1

SUDO=''
[[ $EUID -ne 0 ]] && SUDO=sudo

CUR_DIR=$PWD

function finish() 
{
	cd $CUR_DIR
}
trap finish EXIT

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}
CPUS=${CPUS:-'5'}

# Install DPDK
DPDK_VER=${DPDK_VER:-'18.02.2'}
export RTE_SDK=${RTE_SDK:-$DEPS_DIR/dpdk}
export RTE_TARGET=${RTE_TARGET:-'x86_64-native-linuxapp-gcc'}

install_dpdk() 
{

	[ -d $RTE_SDK ] && echo "DPDK already exists at $RTE_SDK" && return

	echo "Current direcrtory $PWD"
	echo "RTE_SDK = $RTE_SDK "
	mkdir -p ${RTE_SDK} && cd ${RTE_SDK}/../
	wget http://fast.dpdk.org/rel/dpdk-${DPDK_VER}.tar.xz
	tar -xvf dpdk-${DPDK_VER}.tar.xz -C ${RTE_SDK} --strip-components 1

	echo "Applying AVX not supported patch for resolved dpdk-18.02 i40e driver issue.."
	patch $RTE_SDK/drivers/net/i40e/i40e_rxtx.c $RTE_SDK/../../patches/avx_not_suported.patch
	if [ $? -ne 0 ] ; then
		echo "Failed to apply AVX patch, please check the errors."
		return
	fi

	cd ${RTE_SDK}
	#cp $NGIC_CORECUR_DIR/dpdk-18.02_common_linuxapp config/common_linuxapp
	cp $NGIC_DIR/dpdk-18.02_common_linuxapp config/common_linuxapp
#	sed -ri 's,(KNI_KMOD=).*,\1n,' config/common_linuxapp
	make -j $CPUS install T=${RTE_TARGET}
	echo "Installed DPDK at $RTE_SDK"

}

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

