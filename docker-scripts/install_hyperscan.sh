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

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}

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

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

