#! /bin/bash

# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

NG_CORE=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
RTE_SDK=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/third_party/dpdk
HYPERSCAN_DIR="$(pwd)/third_party/hyperscan-4.1.0"
HYPERSCANDIR="$(pwd)/third_party/hyperscan-4.1.0"

export NG_CORE=$NG_CORE
export RTE_SDK=$RTE_SDK
export RTE_TARGET=x86_64-native-linuxapp-gcc
export HYPERSCANDIR=$HYPERSCANDIR

if [[ -d "$HYPERSCAN_DIR" ]]; then
  export HYPERSCANDIR=$HYPERSCAN_DIR
fi

