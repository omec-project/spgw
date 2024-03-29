# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Sprint
#
# SPDX-License-Identifier: Apache-2.0

ROOT=.

CC := gcc

TARGET_DIR := $(ROOT)

SRCDIR := $(ROOT)/src
TARGET := gx_app
BUILDDIR := $(ROOT)/obj

SRCEXT := c

SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
SOURCES += ../../ipc_api.c

OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

#CFLAGS +=  -fPIC -Wall
CFLAGS +=  -fPIC

DEBUG = 1

ifeq ($(DEBUG),1)
	CFLAGS += -g3
#un-comment below line to remove all Debug logs for testing.
	CFLAGS += -DGX_DEBUG $(INCS)
else
	CFLAGS += -O3
endif

INCS := -I$(ROOT)/include -I$(ROOT)/../ -I$(ROOT)/../../../cp/pfcp_messages/ -I$(ROOT)/../../../cp -I$(ROOT)/../../../cpplib
LIB_PATH :=/usr/local/lib
LIBS := -lfdproto -lfdcore

LFLAGS :=

$(TARGET_DIR)/$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGET_DIR)
	@echo $(TARGET_DIR)

	$(CC) $(LFLAGS) $^ -o $(TARGET_DIR)/$(TARGET) -L$(LIB_PATH) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INCS) -c -o $@ $<"; $(CC) $(CFLAGS) $(INCS) -c -o $@ $<

all:$(TARGET)

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET_DIR)/gx_app"; $(RM) -r $(BUILDDIR) $(TARGET_DIR)/gx_app

install:
	@echo " Installing gx-app ";
	@mkdir -p $(TARGET_DIR)/bin
	-@cp $(TARGET) $(TARGET_DIR)/bin
	-@cp config/make_certs.sh $(TARGET_DIR)/bin


.PHONY: clean
