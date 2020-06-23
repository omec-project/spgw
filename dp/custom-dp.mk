# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ZMQ_COMM flag is set for direct ZMQ communication, unset for direct UDP communication
#ZMQ communication is enabled by default
#CFLAGS += -DZMQ_COMM

# ASR- Un-comment below line to shrink pipeline COREs used
CFLAGS += -DNGCORE_SHRINK
