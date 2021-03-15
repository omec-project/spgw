#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

source setenv.sh

build_fd_lib()
{
    echo $DEPS_DIR 
    mkdir -p $DEPS_DIR && pushd $DEPS_DIR
    git clone $FREEDIAMETER
    if [ $? -ne 0 ] ; then
        echo "Failed to clone FreeDiameter, please check the errors."
        return
    fi

	pushd freediameter
	if [ ! -e "build" ]; then
		mkdir build
	fi
	pushd build
	cmake ../
	make || { echo -e "\nFD: Make lib failed\n"; }
	make install || { echo -e "\nFD: Make install failed\n"; }

	libfdproto="/usr/local/lib/libfdproto.so"
	libfdcore="/usr/local/lib/libfdcore.so"

	if [ ! -e "$libfdproto" ]  && [ ! -e "$libfdcore" ]; then
     	        echo "LibFdproto and LibfdCore.so does not exist at /usr/local/lib"
		return
	fi
	popd
	popd
	popd
}

build_gxapp()
{
	pushd $CUR_DIR/cp/gx/gx_app
	make clean
	make || { echo -e "\nGxApp: Make GxApp failed\n"; }
    make install
	popd
}

build_pfcp_lib()
{
        echo "Building libpfcp..."
        pushd $CUR_DIR/libpfcp
        make clean
        make || { echo -e "\nLibPfcp: Make lib failed\n"; }
        popd
}

build_libgtpcv2c()
{

        echo "Building libgtpv2c..."
        pushd $CUR_DIR/libgtpv2c
        make clean
        make || { echo -e "\nlibgtpv2c: Make failed\n"; }
        popd

}

build_fd_gxapp()
{
	echo "Building FreeDiameter ..."
	build_fd_lib
	ldconfig
	echo "Building GxAPP ..."
	build_gxapp
}

build_c3po_util()
{
	echo "Building c3po util ..."
	pushd $SPGW_DIR/$C3PO_OSS_DIR/$OSS_UTIL_DIR
	make clean
	make
	$SUDO make install
	popd
}

build_cpputil_lib()
{
	echo "Building cpp util ..."
	pushd $SPGW_DIR/cpplib
	make clean
	make
	$SUDO make install
	ls -l $SPGW_DIR/cpplib/target/lib
	popd
}

build_spgw()
{
	pushd $SPGW_DIR
	source setenv.sh
	build_cpputil_lib
	#build_c3po_util
	build_pfcp_lib

	#make build-lib || { echo -e "\nmake lib failed\n"; }

	echo "Building libgtpv2c..."
	pushd $SPGW_DIR/libgtpv2c
	make 
	if [ $? -ne 0 ] ; then
		echo "Failed to build libgtpv2, please check the errors."
		return
	fi
	popd
	
	echo "Building FD and GxApp..."
	build_fd_gxapp
	
	echo "Building CP..."
	make clean-cp
	make -j 10 build-cp RTE_MACHINE=$RTE_MACHINE EXTRA_CFLAGS="$EXTRA_CFLAGS" || { echo -e "\nCP: Make failed\n"; }
	popd
}

(return 2>/dev/null) && echo "Sourced" && return
