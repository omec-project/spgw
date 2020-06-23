# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#SDN_ODL_BUILD flag is set for ODL builds, unset for direct UDP or ZMQ[Direct || ODL] communication between CP and DP
#CFLAGS += -DSDN_ODL_BUILD

#Enable/Disable below flag to enable /disable CLI and Logger(OSS-UTILS)
CFLAGS += -DC3PO_OSS 

#Enable/Disable multi UPF. If multi UPF is disabled then cp.cfg should have fixed UPF address  
CFLAGS += -DMULTI_UPFS 
