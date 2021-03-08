#! /bin/bash

# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

NG_CORE=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
RTE_SDK=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/third_party/dpdk

export NG_CORE=$NG_CORE
export RTE_SDK=$RTE_SDK
export RTE_TARGET=x86_64-native-linuxapp-gcc
