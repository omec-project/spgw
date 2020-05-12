#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

source ./git_url.cfg
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

install_pkg_deps() 
{
	$SUDO apt-get update && $SUDO apt-get -y install \
		curl build-essential \
		linux-headers-$(uname -r) git \
        unzip libpcap0.8-dev gcc libjson0-dev make \
        libc6 libc6-dev \
        g++-multilib libcurl4-openssl-dev \
		libssl-dev \
        python-pip \
		wget \
        lsb-core \
        g++ \
        make \
        cmake \
        libuv-dev \
        libssl-dev autotools-dev libtool-bin m4 automake libmemcached-dev \
        memcached \
        cmake-curses-gui \
        gcc\
        bison flex libsctp-dev libgnutls-dev libgcrypt-dev libidn11-dev \
		nettle-dev \
		ragel

       download_hyperscan
}

#OSDIST=`lsb_release -is`
#OSVER=`lsb_release -rs`

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

download_hyperscan()
{
    [ -d $DEPS_DIR/hyperscan-4.1.0 ] && echo "Hyperscan already exists at $DEPS_DIR/hyperscan-4.1.0" && return


    mkdir -p $DEPS_DIR
    cd $DEPS_DIR
    source /etc/os-release
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
}

install_hyperscan()
{
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
		echo "LD library path $LD_LIBRARY_PATH"
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

build_c_ares()
{
  pushd modules/c-ares
  ./buildconf
  ./configure
  make
  $SUDO make install
  popd
  
  BUILD_C_ARES_COMPLETE="- COMPLETE"
}

build_cpp_driver()
{
  pushd modules/cpp-driver
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_CPP_DRIVER_COMPLETE="- COMPLETE"
}

build_pistache()
{
  pushd modules/pistache
  rm -rf build
  mkdir -p build
  cd build
  cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
  make
  $SUDO make install
  popd
  
  BUILD_PISTACHE_COMPLETE="- COMPLETE"
}

build_rapidjson()
{
  pushd modules/rapidjson
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_RAPIDJSON_COMPLETE="- COMPLETE"
}

build_spdlog()
{
  pushd modules/spdlog
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_SPDLOG_COMPLETE="- COMPLETE"
}

build_cli()
{
  pushd cli
  $SUDO apt-get -y install python-pip
  $SUDO pip install -r requirements.txt
  $SUDO apt-get -y install python-virtualenv
  virtualenv -p python3.5 venv
  set +u
  source venv/bin/activate
  set -u
  $SUDO pip install -r requirements.txt
  deactivate
  popd
}

init_oss_util_submodules()
{
  git submodule init
  git submodule update

  build_c_ares
  build_cpp_driver
  build_pistache
  build_rapidjson
  build_spdlog
  build_cli 
 
  $SUDO ldconfig
}

#build_c3po_util()
#{
#  make clean
#  $SUDO make install
#}

install_oss_util()
{
   mkdir -p $NGIC_DIR/$C3PO_OSS_DIR
   pushd $NGIC_DIR/$C3PO_OSS_DIR
   git clone $OSS_UTIL_GIT_LINK
   pushd oss-util
   ./install_builddeps_oss.sh
   init_oss_util_submodules
   #build_c3po_util
   popd
   popd
}

install_build_deps()
{
       install_pkg_deps
       download_hyperscan
       install_dpdk
       download_freediameter
       install_oss_util
	   install_hyperscan
}

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

install_build_deps
echo "Dependency install complete"

