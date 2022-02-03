# Copyright 2020-present Open Networking Foundation
#
# SPDX-License-Identifier: Apache-2.0

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
TOPDIR = $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))

TARGET_DIR = $(TOPDIR)/target
BUILDDIR = $(TOPDIR)/build
BINDIR = $(BUILDDIR)/bins
OBJDIR = $(BUILDDIR)/objs
LIBDIR = $(BUILDDIR)/libs

CFLAGS += -D_FORTIFY_SOURCE=2 -fasynchronous-unwind-tables -fexceptions
CFLAGS += -fstack-protector-all -fstack-protector-strong -Wall
CFLAGS += -Werror=format-security 


CC := g++
CFLAGS += -std=c++14

DEBUG = true
ifeq ($(DEBUG),true)
        CFLAGS += -g
endif
ifeq ($(DEBUG),false)
        CFLAGS += -O3
endif

SPGW_STATIC_LIBNAME = libspgwcpputil.a 

SPGWLIB_SRCS := ./spgw_config.cpp \
				./spgw_tables.cpp \
				./spgwStatsPromClient.cpp \
				./spgw_wrapper.cpp \
				./spgw_webserver.cpp \
				./delayed_task.cpp  
                 

SRCDIR := .
SRCEXT := cpp
SPGW_CPP_OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SPGWLIB_SRCS:.$(SRCEXT)=.o))

OBJS := $(SPGW_CPP_OBJS)

all : buildCommonLIBS install

buildCommonLIBS : $(OBJS)
	@echo "Linking"
	@mkdir -p $(LIBDIR)
	ar rvs $(SPGW_STATIC_LIBNAME) $(SPGW_CPP_OBJS) 


$(OBJS) : $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/
	$(CC) $(CFLAGS) $(INC_DIRS) -fPIC -c -o $@ $<

install:
	@echo "Installing"
	-@mkdir -p $(TARGET_DIR)/lib
	-@cp $(SPGW_STATIC_LIBNAME) $(TARGET_DIR)/lib
	-@ls -l $(TARGET_DIR)/lib

clean:
	@echo " Cleaning...";
	-@rm -rf a.out
	-@rm -rf $(OBJDIR)/
	-@rm -f $(SPGW_STATIC_LIBNAME)
	-@rm -f $(TARGET_DIR)/lib/$(SPGW_STATIC_LIBNAME)

test: 
	gcc -g test_main.c -L/users/ajayonf/spgw.ajay/cpplib -lspgwcpputil

