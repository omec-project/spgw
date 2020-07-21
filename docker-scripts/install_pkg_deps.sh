#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"

export SPGW_DIR=$PWD

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
        g++-multilib \
        libcurl4-openssl-dev \
        libssl-dev \
        python-pip \
        wget \
        lsb-core \
        g++ \
        make \
        cmake \
        libuv-dev \
        libssl-dev \
        autotools-dev \
        libtool-bin \
        m4 \
        automake \
        libmemcached-dev \
        memcached \
        cmake-curses-gui \
        gcc\
        bison flex libsctp-dev libgnutls-dev libgcrypt-dev libidn11-dev \
        nettle-dev \
        ragel \
        libmnl-dev \
        rapidjson-dev

}

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}
CPUS=${CPUS:-'5'}

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

install_build_deps
echo "Dependency install complete"

