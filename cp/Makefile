# Copyright 2020-present Open Networking Foundation
# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

MAKEFLAGS += -j
# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk
include $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/*.mk

OSS_UTIL_ROOT = $(NG_CORE)/oss_adapter/c3po_oss/oss-util
LIBGTPV2C_ROOT = $(SRCDIR)/../libgtpv2c
LIBPFCP_ROOT = $(SRCDIR)/../libpfcp
CPPUTILLIB_ROOT = $(SRCDIR)/../cpplib/target
$(info VAR is $(CPPUTILLIB_ROOT))

# binary name
APP = ngic_controlplane

# all source are stored in SRCS-y

SRCS-y := cp_main.c
SRCS-y += ue.c
SRCS-y += ip_pool.c
SRCS-y += cp_init.c
SRCS-y += cp_stats.c
SRCS-y += monitor_config.c
SRCS-y += cp_config.c
SRCS-y += packet_filters.c
SRCS-y += cp_interface.c
SRCS-y += cp_io_poll.c
SRCS-y += cp_peer.c
SRCS-y += cp_timer.c
SRCS-y += cp_transactions.c
SRCS-y += cp_session.c
SRCS-y += ipc_api.c
SRCS-y += state_machine/sm_init.o
SRCS-y += state_machine/sm_upf.o
SRCS-y += state_machine/sm_error_handlers.o
SRCS-y += tables/upf_table.o
SRCS-y += tables/ue_table.o
SRCS-y += tables/pdn_table.o
SRCS-y += tables/bearer_table.o
SRCS-y += tables/peer_nodes.o
SRCS-y += tables/pfcp_heartbeat.o
SRCS-y += tables/gx_context_table.o
SRCS-y += tables/callid_table.o
SRCS-y += procedures/report_proc.o
SRCS-y += procedures/initial_attach_proc.o
SRCS-y += procedures/pfcp_association_setup_proc.o
SRCS-y += procedures/rab_proc.o
SRCS-y += procedures/detach_proc.o
SRCS-y += procedures/service_request_proc.o
SRCS-y += procedures/bearer_create_proc.o
SRCS-y += procedures/bearer_update_proc.o
SRCS-y += procedures/bearer_delete_proc.o
SRCS-y += procedures/pfcp_set_update_proc.o
SRCS-y += procedures/pfd_proc.o
SRCS-y += procedures/sgw_reloc_proc.o
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_error_rsp.c
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_interface.c
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_set_ie.c
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_msg_rx.c
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_utils.c
SRCS-y += $(SRCDIR)/../gtpv2/common/gtpv2_msg_validate.c
SRCS-y += $(SRCDIR)/../gtpv2/msg/gtpv2_echo.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/create_session_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/create_session_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/modify_bearer_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/modify_bearer_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_session_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_session_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/create_bearer_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/create_bearer_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/update_bearer_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/update_bearer_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_bearer_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_bearer_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/downlink_data_notification.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/release_access_bearer.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/bearer_resource_cmd.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_bearer_command.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_pdn_conn_set_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/delete_pdn_conn_set_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/update_pdn_conn_set_request.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/update_pdn_conn_set_response.o
SRCS-y += $(SRCDIR)/../gtpv2/msg/pgw_restart_notification_ack.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_heartbit.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_cp_association.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_cp_msg_rx.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_cp_session.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_cp_set_ie.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_cp_util.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_init.o
SRCS-y += $(SRCDIR)/../pfcp/common/pfcp_interface.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_association_setup_request.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_association_setup_response.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_session_est_response.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_session_delete_response.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_session_mod_response.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_session_report.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_set_deletion_response.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfcp_set_deletion_request.o
SRCS-y += $(SRCDIR)/../pfcp/msg/pfd_management_response.o
SRCS-y += $(SRCDIR)/../oss_adapter/c3po_oss/gw_adapter.o
SRCS-y += $(SRCDIR)/../gx/gx_error_rsp.c
SRCS-y += $(SRCDIR)/../gx/gx_interface.c
SRCS-y += $(SRCDIR)/../gx/pfcp_gx.o
SRCS-y += $(SRCDIR)/../gx/msg/cca_initial_handler.o
SRCS-y += $(SRCDIR)/../gx/msg/cca_terminate_handler.o
SRCS-y += $(SRCDIR)/../gx/msg/cca_update_handler.o
SRCS-y += $(SRCDIR)/../gx/msg/rar_handler.o
SRCS-y += $(SRCDIR)/../gx/gx_app/src/gx_pack.o



#ifneq (,$(findstring C3PO_OSS, $(CFLAGS)))
#SRCS-y += $(SRCDIR)/../oss_adapter/c3po_oss/cp_adapter.o
#CFLAGS += -I$(SRCDIR)/../oss_adapter/c3po_oss
#endif

CFLAGS += -Wno-psabi # suppress "The ABI for passing parameters with 64-byte alignment has changed in GCC 4.6"
CFLAGS += -Werror
CFLAGS += -Wunused-variable

CFLAGS += -I$(SRCDIR)/..
CFLAGS += -I$(SRCDIR)/../common
CFLAGS += -I$(SRCDIR)/../cp
CFLAGS += -I$(SRCDIR)/../cp/gx_app/include
CFLAGS += -I$(SRCDIR)/../cp_dp_api
CFLAGS += -I$(SRCDIR)/../gtpv2/common
CFLAGS += -I$(SRCDIR)/procedures
CFLAGS += -I$(SRCDIR)/state_machine
CFLAGS += -I$(SRCDIR)/../gx/gx_app/include/
CFLAGS += -I$(LIBGTPV2C_ROOT)/include
CFLAGS += -I$(LIBPFCP_ROOT)/include
CFLAGS += -I$(SRCDIR)/../oss_adapter/c3po_oss
CFLAGS += -I$(SRCDIR)/../gx
CFLAGS += -I$(SRCDIR)/../pfcp/common
CFLAGS += -I$(SRCDIR)/../libgtpv2c/include
CFLAGS += -I$(OSS_UTIL_ROOT)/include
CFLAGS += -I$(SRCDIR)/../cpplib/

# Mandatory CFLAGS, LDFLAGS- DO NOT MODIFY
# #############################################################
CFLAGS += -DCP_BUILD

# workaround for a gcc bug with noreturn attribute
#  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=12603
ifeq ($(CONFIG_RTE_TOOLCHAIN_GCC),y)
CFLAGS_controlplane.o += -Wno-return-type
endif

LDLIBS += -L$(LIBGTPV2C_ROOT)/lib -lgtpv2c
LDLIBS += -L$(LIBPFCP_ROOT)/lib -lpfcp
LDLIBS += -L$(CPPUTILLIB_ROOT)/lib -lspgwcpputil
$(info LDLIBS is $(LDLIBS))
LDLIBS += -lpcap
LDFLAGS += -lstdc++
LDLIBS += -lstdc++
#LDLIBS +=  -L$(OSS_UTIL_ROOT)/lib -loss-util
LDLIBS += -lpthread
LDLIBS += -lrt
LDLIBS += -lpistache
LDLIBS += -lm

# ngic-cp debug/testing/profiling options CFLAGS
# #############################################################
#Enable'-g -O0' option for gdb
#CFLAGS += -O0
CFLAGS += -g -O0

#un-comment below line to remove all log level for testing.
CFLAGS += -DPERF_TEST

#Enable '-E' option to debug preprocessor output
#CFLAGS += -E

CFLAGS += $(WERROR_FLAGS)
CFLAGS_config.o := -D_GNU_SOURCE

#un-comment below line to generate req and resp timer stats in between
#CP<-->DP(ZMQ) and CP<-->FPC<-->DP.
#Note: Req and Resp timer stats is not supported for CP <--> DP direct comm over UDP.
#CFLAGS += -DSYNC_STATS

ifneq (,$(findstring SYNC_STATS, $(CFLAGS), $(EXTRA_CFLAGS)))
	SRCS-y += stats_sync.c
endif

#un-comment below line to enable FQ-CSID support in the control-plane
#CFLAGS += -DUSE_CSID

ifneq (,$(findstring USE_CSID, $(CFLAGS), $(EXTRA_CFLAGS)))
 #VS:Support partial failure FQ-CSID m/c

 SRCS-y += $(SRCDIR)/../pfcp/common/csid_init.o
 SRCS-y += $(SRCDIR)/../pfcp/common/csid_api.o
 SRCS-y += $(SRCDIR)/../pfcp/common/csid_peer_init.o
 SRCS-y += $(SRCDIR)/../pfcp/common/csid_cp_cleanup.o

endif

# Un-comment below line to read fake cp config.
#CFLAGS += -DSIMU_CP

# Un-comment below line to send fake delete session request from simulator.
#CFLAGS += -DDEL_SESS_REQ

# Comment below line to read UPF ip from cp.cfg instead of DNS query.
#ajay - add ticket to support DNS in SPGW 
#CFLAGS += -DUSE_DNS_QUERY

#ifneq (,$(findstring USE_DNS_QUERY, $(CFLAGS)))
	LIBDNSCLIENT_ROOT = $(SRCDIR)/../oss_adapter/c3po_oss/oss-util
	CFLAGS += -I$(LIBDNSCLIENT_ROOT)/include
	LDLIBS += $(LIBDNSCLIENT_ROOT)/lib/liboss-util.a -lstdc++ -lpistache
	LDLIBS += -L$(LIBDNSCLIENT_ROOT)/modules/c-ares -lcares
#endif

# ngic-cp application security check CFLAGS
###############################################################
SECURITY_FLAGS = -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fexceptions  -fpie -fstack-protector-all -fstack-protector-strong -Wall -Werror=format-security -Werror=implicit-function-declaration

CFLAGS += $(SECURITY_FLAGS)

# ngic-cp include make overlays
# #############################################################
include $(RTE_SDK)/mk/rte.extapp.mk

