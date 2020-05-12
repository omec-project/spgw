#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"

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

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}
CPUS=${CPUS:-'5'}

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

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

install_build_deps
echo "Dependency install complete"

