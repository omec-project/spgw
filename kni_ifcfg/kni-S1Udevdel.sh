#! /bin/bash

# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

# Move to script directory
cd $(dirname ${BASH_SOURCE[0]})

source ../config/dp_config.cfg
ifconfig $UL_IFACE
ip addr del $S1U_IP/24 dev $UL_IFACE
ifconfig $UL_IFACE

