# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(HYPERSCANDIR),)
$(error "Please define HYPERSCANDIR environment variable")
endif

$(info $$HYPERSCANDIR is [${HYPERSCANDIR}])

include $(RTE_SDK)/mk/rte.vars.mk

# library name
LIB = libsponsdn.a

CFLAGS += -O3
CFLAGS += $(WERROR_FLAGS) -I$(SRCDIR) -I$(HYPERSCANDIR)/src


# all source are stored in SRCS-y
SRCS-y += sponsdn.c


include $(RTE_SDK)/mk/rte.extlib.mk
