#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

source ./git_url.cfg
THIRD_PARTY_SW_PATH="third_party"
OSS_UTIL_DIR="oss-util"
C3PO_OSS_DIR="oss_adapter/c3po_oss"

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

DEPS_DIR=${DEPS_DIR:-"$PWD/$THIRD_PARTY_SW_PATH"}

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

(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

install_build_deps
echo "Dependency install complete"

