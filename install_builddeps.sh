#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

source ./git_url.cfg
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

install_pkg_deps() {
	$SUDO apt-get update && $SUDO apt-get -y install \
		curl build-essential \
		linux-headers-$(uname -r) git \
        unzip libpcap0.8-dev gcc libjson0-dev make \
        libc6 libc6-dev \
        g++-multilib libcurl4-openssl-dev \
		libssl-dev \
        python-pip \
		wget \
        lsb-core
}

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}
CPUS=${CPUS:-'5'}

# Install DPDK
DPDK_VER=${DPDK_VER:-'18.02.2'}
export RTE_SDK=${RTE_SDK:-$DEPS_DIR/dpdk}
export RTE_TARGET=${RTE_TARGET:-'x86_64-native-linuxapp-gcc'}

install_dpdk() {

	[ -d $RTE_SDK ] && echo "DPDK already exists at $RTE_SDK" && return

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
	cp $CUR_DIR/dpdk-18.02_common_linuxapp config/common_linuxapp
#	sed -ri 's,(KNI_KMOD=).*,\1n,' config/common_linuxapp
	make -j $CPUS install T=${RTE_TARGET}
	echo "Installed DPDK at $RTE_SDK"

}

download_hyperscan()
{
    [ -d $DEPS_DIR/hyperscan-4.1.0 ] && echo "Hyperscan already exists at $DEPS_DIR/hyperscan-4.1.0" && return


    $SUDO apt-get -y install ragel
    cd $DEPS_DIR
    source /etc/os-release
    cd $DEPS_DIR
    if [[ $VERSION_ID != "16.04" ]] ; then
        echo "Download boost manually "$VERSION_ID
        wget http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz
        tar -xf boost_1_58_0.tar.gz
        pushd boost_1_58_0
        $SUDO apt-get -y install g++
        ./bootstrap.sh --prefix=/usr/local
        ./b2
        ./b2 install
        popd
    else
        $SUDO apt-get -y install libboost-all-dev
    fi

        echo "Downloading HS and dependent libraries"
        wget $HYPERSCAN_GIT_LINK
        tar -xvf v4.1.0.tar.gz
        pushd hyperscan-4.1.0
        mkdir build; pushd build
        cmake -DCMAKE_CXX_COMPILER=c++ ..
        cmake --build .
        export LD_LIBRARY_PATH=${LD_LIBRARY_PATH-}:$PWD/lib
        popd
        export HYPERSCANDIR=$PWD
        echo "export HYPERSCANDIR=$PWD" >> ../setenv.sh
        popd
}

download_freediameter()
{
        cd $CUR_DIR
        [ -d $DEPS_DIR/freediameter ] && echo "FreeDiameter already exists at $DEPS_DIR/freediameter" && return
        echo "Download FreeDiameter from sprint-repos....."
        if [ ! -d $THIRD_PARTY_SW_PATH ]; then
             mkdir $THIRD_PARTY_SW_PATH
        fi
        pushd $THIRD_PARTY_SW_PATH
        git clone $FREEDIAMETER
        if [ $? -ne 0 ] ; then
                        echo "Failed to clone FreeDiameter, please check the errors."
                        return
        fi
        popd

}

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

install_oss_util()
{
   pushd $NGIC_DIR/$C3PO_OSS_DIR
   git clone $OSS_UTIL_GIT_LINK
   pushd oss-util
   cp $NGIC_DIR/install_builddeps_oss.sh .
   ./install_builddeps_oss.sh
   popd
   popd
}

install_build_deps()
{
       install_pkg_deps
       install_oss_util
       install_dpdk
       download_hyperscan
       download_freediameter
}

build_ngic()
{
	pushd $NGIC_DIR
	source setenv.sh

	echo "Building PFCP Libs ..."
	build_pfcp_lib

	if [ $SERVICE == 2 ] || [ $SERVICE == 3 ] ; then
		make clean-lib
		make clean-dp
		echo "Building Libs..."
		make build-lib || { echo -e "\nNG-CORE: Make lib failed\n"; }
		echo "Building DP..."
		make build-dp || { echo -e "\nDP: Make failed\n"; }
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

install_build_deps
build_ngic
echo "Dependency install complete"

