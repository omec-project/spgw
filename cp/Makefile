# Copyright 2020-present Open Networking Foundation
# Copyright (c) 2017 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
ROOT=.

CC := g++

TARGET_DIR := $(ROOT)/bin

SRCDIR := $(ROOT)/

BUILDDIR := $(ROOT)/obj

LIBGTPV2C_ROOT = $(SRCDIR)/../libgtpv2c
LIBPFCP_ROOT = $(SRCDIR)/../libpfcp
CPPUTILLIB_ROOT = $(SRCDIR)/../cpplib/target

# binary name
APP = ngic_controlplane

# all source are stored in SOURCES

SOURCES := cp_main.o
SOURCES += cp_test.o
SOURCES += ue.o
SOURCES += pdn.o
SOURCES += bearer.o
SOURCES += ip_pool.o
SOURCES += cp_init.o
SOURCES += monitor_config.o
SOURCES += cp_config.o
SOURCES += packet_filters.o
SOURCES += cp_interface.o
SOURCES += cp_io_poll.o
SOURCES += cp_peer.o
SOURCES += cp_timer.o
SOURCES += cp_transactions.o
SOURCES += cp_session.o
SOURCES += ipc_api.o
SOURCES += upf.o
SOURCES += sm_error_handlers.o
SOURCES += procedures/proc.o
SOURCES += procedures/proc_session_report.o
SOURCES += procedures/proc_initial_attach.o
SOURCES += procedures/proc_pfcp_assoc_setup.o
SOURCES += procedures/proc_s1_release.o
SOURCES += procedures/proc_nw_init_detach.o
SOURCES += procedures/proc_detach.o
SOURCES += procedures/proc_service_request.o
SOURCES += procedures/proc_bearer_create.o
SOURCES += procedures/proc_bearer_update.o
SOURCES += procedures/proc_bearer_delete.o
SOURCES += procedures/proc_pfd_mgmt.o
SOURCES += procedures/proc_sgw_relocation.o
SOURCES += procedures/proc_rar.o
SOURCES += gtpv2/common/gtpv2_error_rsp.o
SOURCES += gtpv2/common/gtpv2_interface.o
SOURCES += gtpv2/common/gtpv2_set_ie.o
SOURCES += gtpv2/common/gtpv2_msg_rx.o
SOURCES += gtpv2/common/gtpv2_utils.o
SOURCES += gtpv2/common/gtpv2_msg_validate.o
SOURCES += gtpv2/msg/gtpv2_echo.o
SOURCES += gtpv2/msg/create_session_request.o
SOURCES += gtpv2/msg/create_session_response.o
SOURCES += gtpv2/msg/modify_bearer_request.o
SOURCES += gtpv2/msg/delete_session_request.o
SOURCES += gtpv2/msg/create_bearer_response.o
SOURCES += gtpv2/msg/update_bearer_response.o
SOURCES += gtpv2/msg/delete_bearer_response.o
SOURCES += gtpv2/msg/downlink_data_notification.o
SOURCES += gtpv2/msg/release_access_bearer.o
SOURCES += gtpv2/msg/bearer_resource_cmd.o
SOURCES += gtpv2/msg/delete_bearer_command.o
SOURCES += pfcp/common/pfcp_error_rsp.o
SOURCES += pfcp/common/pfcp_cp_association.o
SOURCES += pfcp/common/pfcp_cp_msg_rx.o
SOURCES += pfcp/common/pfcp_cp_session.o
SOURCES += pfcp/common/pfcp_cp_set_ie.o
SOURCES += pfcp/common/pfcp_cp_util.o
SOURCES += pfcp/common/pfcp_init.o
SOURCES += pfcp/common/pfcp_interface.o
SOURCES += pfcp/msg/pfcp_heartbeat.o
SOURCES += pfcp/msg/pfcp_association_setup_request.o
SOURCES += pfcp/msg/pfcp_association_setup_response.o
SOURCES += pfcp/msg/pfcp_session_est_response.o
SOURCES += pfcp/msg/pfcp_session_delete_response.o
SOURCES += pfcp/msg/pfcp_session_mod_response.o
SOURCES += pfcp/msg/pfcp_session_report.o
SOURCES += pfcp/msg/pfd_management_response.o
SOURCES += gx/gx_error_rsp.o
SOURCES += gx/gx_interface.o
SOURCES += gx/pfcp_gx.o
SOURCES += gx/msg/cca_initial_handler.o
SOURCES += gx/msg/cca_terminate_handler.o
SOURCES += gx/msg/cca_update_handler.o
SOURCES += gx/msg/rar_handler.o
SOURCES += gx/gx_app/src/gx_pack.o

CFLAGS += -Wno-psabi # suppress "The ABI for passing parameters with 64-byte alignment has changed in GCC 4.6"
CFLAGS += -Werror

CFLAGS += -I$(LIBGTPV2C_ROOT)/include
CFLAGS += -I$(LIBPFCP_ROOT)/include
CFLAGS += -I./../cpplib/
CFLAGS += -I./
CFLAGS += -I./gx/
CFLAGS += -I./gx/gx_app/include
CFLAGS += -I./gtpv2/common
CFLAGS += -I./pfcp/common
CFLAGS += -I./procedures
CFLAGS += -I$(SRCDIR)/../gx
CFLAGS += -I/usr/include

LDLIBS += -L$(LIBGTPV2C_ROOT)/lib -lgtpv2c
LDLIBS += -L$(LIBPFCP_ROOT)/lib -lpfcp
LDLIBS += -L$(CPPUTILLIB_ROOT)/lib -lspgwcpputil
LDLIBS += -L/usr/local/lib -lprometheus-cpp-pull -lprometheus-cpp-core -lpistache
LDLIBS += -L/usr/local/lib -lpistache
LDLIBS += -lpcap -lcrypto -lssl
LDLIBS += -lstdc++
LDLIBS += -lpthread
LDLIBS += -lrt
LDLIBS += -lm

# ngic-cp debug/testing/profiling options CFLAGS
# #############################################################
#Enable'-g -O0' option for gdb
#CFLAGS += -O0
CFLAGS += -g -O0

#Enable '-E' option to debug preprocessor output
#CFLAGS += -E

CFLAGS += $(WERROR_FLAGS)
CFLAGS_config.o := -D_GNU_SOURCE

LDLIBS +=  -lstdc++

# ngic-cp application security check CFLAGS
###############################################################
SECURITY_FLAGS = -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fexceptions  -fpie -fstack-protector-all -fstack-protector-strong -Wall -Werror=format-security -Werror=implicit-function-declaration

CFLAGS += $(SECURITY_FLAGS)

OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

$(TARGET_DIR)/$(APP): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGET_DIR)
	$(CC) $(LFLAGS) $^ $(LDLIBS) -o $(TARGET_DIR)/ngic_controlplane

#$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
#	@echo " target 1..."
#	@mkdir -p $(BUILDDIR)
#	@echo " $(CC) $(CFLAGS) $(INCS) -c -o $@ $<"; $(CC) $(CFLAGS) $(INCS) -c -o $@ $<

all:$(APP)

.PHONY: clean
