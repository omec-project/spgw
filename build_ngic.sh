#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

export NGIC_DIR=$PWD

SERVICE_NAME="Collocated CP and DP"
SERVICE=3
SUDO=''
[[ $EUID -ne 0 ]] && SUDO=sudo

CUR_DIR=$PWD
function finish() {
	cd $CUR_DIR
}
trap finish EXIT
source setenv.sh


DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}
CPUS=${CPUS:-'5'}

# Install DPDK
DPDK_VER=${DPDK_VER:-'18.02.2'}
export RTE_SDK=${RTE_SDK:-$DEPS_DIR/dpdk}
export RTE_TARGET=${RTE_TARGET:-'x86_64-native-linuxapp-gcc'}



build_fd_lib()
{
	pushd $CUR_DIR/$THIRD_PARTY_SW_PATH/freediameter
	if [ ! -e "build" ]; then
		mkdir build
	fi
	pushd build
	cmake ../
	make || { echo -e "\nFD: Make lib failed\n"; }
	make install || { echo -e "\nFD: Make install failed\n"; }

	libfdproto="/usr/local/lib/libfdproto.so"
	libfdcore="/usr/local/lib/libfdcore.so"

	if [ ! -e "$libfdproto" ]  && [ ! -e "$libfdcore" ]; then
     	        echo "LibFdproto and LibfdCore.so does not exist at /usr/local/lib"
		return
	fi
	popd
	popd
}

build_gxapp()
{
	pushd $CUR_DIR/cp/gx_app
	make clean
	make || { echo -e "\nGxApp: Make GxApp failed\n"; }
	popd
}

build_pfcp_lib()
{
        echo "Building libpfcp..."
        pushd $CUR_DIR/libpfcp
        make clean
        make || { echo -e "\nLibPfcp: Make lib failed\n"; }
        popd
}

build_libgtpcv2c(){

        echo "Building libgtpv2c..."
        pushd $CUR_DIR/libgtpv2c
        make clean
        make || { echo -e "\nlibgtpv2c: Make failed\n"; }
        popd

}

build_fd_gxapp()
{
	echo "Building FreeDiameter ..."
	build_fd_lib
	ldconfig

	echo "Building GxAPP ..."
	build_gxapp
}

build_c3po_util()
{
  echo "Building c3po util ..."
  pushd $NGIC_DIR/$C3PO_OSS_DIR/$OSS_UTIL_DIR
  make clean
  make
  $SUDO make install
  popd
}


build_ngic()
{
	pushd $NGIC_DIR
	source setenv.sh

    build_c3po_util
	build_pfcp_lib

	if [ $SERVICE == 2 ] || [ $SERVICE == 3 ] ; then
		make clean-dp
		echo "Building Libs..."
		make build-lib || { echo -e "\nNG-CORE: Make lib failed\n"; }
		echo "Building DP..."
		### USE_AF_PACKET for deploying in k8s
		### ggdb must be made standard to help debugging
		### O2 because O3 causes DP crash https://github.com/omec-project/ngic-rtc/issues/55
		make build-dp EXTRA_CFLAGS='-DUSE_AF_PACKET -ggdb -O2' || { echo -e "\nDP: Make failed\n"; }
	fi
	if [ $SERVICE == 1 ] || [ $SERVICE == 3 ] ; then
		echo "Building libgtpv2c..."
		pushd $NGIC_DIR/libgtpv2c
			make clean
			make
			if [ $? -ne 0 ] ; then
				echo "Failed to build libgtpv2, please check the errors."
				return
			fi
		popd

		echo "Building FD and GxApp..."
		build_fd_gxapp

		echo "Building CP..."
		make clean-cp
		make -j 10 build-cp || { echo -e "\nCP: Make failed\n"; }

	fi
	popd
}


(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

build_ngic
echo "NGIC build complete "

