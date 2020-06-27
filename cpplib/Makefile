#
# Copyright (c) 2019, Infosys Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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
CFLAGS += -std=c++11 

DEBUG = false
ifeq ($(DEBUG),true)
        CFLAGS += -g
endif
ifeq ($(DEBUG),false)
        CFLAGS += -O3
endif

SPGW_LIBNAME = libspgwcpputil.so 

SPGWLIB_SRCS := ./spgw_config.cpp \
                   ./spgw_wrapper.cpp 

SRCDIR := .
SRCEXT := cpp
SPGW_CPP_OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SPGWLIB_SRCS:.$(SRCEXT)=.o))

OBJS := $(SPGW_CPP_OBJS)

all : buildCommonLIBS install

buildCommonLIBS : $(OBJS)
	@echo "Linking"
	@mkdir -p $(LIBDIR)
	$(CC) $(CFLAGS) $(SPGW_CPP_OBJS) -shared -o $(SPGW_LIBNAME)

$(OBJS) : $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/
	$(CC) $(CFLAGS) $(INC_DIRS) -fPIC -c -o $@ $<

install:
	@echo "Installing"
	-@mkdir -p $(TARGET_DIR)/lib
	-@cp $(SPGW_LIBNAME) $(TARGET_DIR)/lib

clean:
	@echo " Cleaning...";
	-@rm -rf $(OBJDIR)/
	-@rm -f $(SPGW_LIBNAME)
	-@rm -f $(TARGET_DIR)/lib/$(SPGW_LIBNAME)

