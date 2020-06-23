#! /bin/bash

# SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

source ../config/dp_config.cfg

if [ "$SPGW_CFG" -eq "01" ]; then
	ifconfig $S5S8_IFACE
	ip addr del $S5S8_SGWU_IP/24 dev $S5S8_IFACE
elif [ "$SPGW_CFG" -eq "02" ];
then
	ifconfig $S5S8_IFACE
	ip addr del $S5S8_PGWU_IP/24 dev $S5S8_IFACE
else
	echo "Error: Set appropriate data-plane type.."
	exit
fi

ifconfig $S5S8_IFACE
