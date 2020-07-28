#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

export SPGW_DIR=$PWD

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

build_libgtpcv2c()
{

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
  pushd $SPGW_DIR/$C3PO_OSS_DIR/$OSS_UTIL_DIR
  make clean
  make
  $SUDO make install
  popd
}

build_cpputil_lib()
{
  echo "Building cpp util ..."
  pushd $SPGW_DIR/cpplib
  make clean
  make
  $SUDO make install
  ls -l $SPGW_DIR/cpplib/target/lib
  popd
}

build_spgw()
{
	pushd $SPGW_DIR
	source setenv.sh
    build_cpputil_lib
   	build_c3po_util
	build_pfcp_lib

	if [ $SERVICE == 2 ] || [ $SERVICE == 3 ] ; then
		echo "Building Libs..."
		make build-lib || { echo -e "\nmake lib failed\n"; }
		echo "Building DP..."
		#make build-dp -j 10 EXTRA_CFLAGS='-DUSE_AF_PACKET -ggdb -O2' || { echo -e "\nmake failed\n"; }
	fi
	if [ $SERVICE == 1 ] || [ $SERVICE == 3 ] ; then
		echo "Building libgtpv2c..."
		pushd $SPGW_DIR/libgtpv2c
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
		make -j 10 build-cp EXTRA_CFLAGS='-ggdb -O2 ' || { echo -e "\nCP: Make failed\n"; }
	fi
	popd
}

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset
