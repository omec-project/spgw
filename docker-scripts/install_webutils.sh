#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

build_pistache()
{
    set -xe
    pushd /tmp
    git clone -q $PISTACHE_SERVER /tmp/pistache
    pushd /tmp/pistache
    git checkout 9dc080b9ebbe6fc1726b45e9db1550305938313e
    git submodule update --init
    mkdir -p {build,installpath}
    cd build
    /tmp/cmake-3.18.0-Linux-x86_64/bin/cmake --version
    /tmp/cmake-3.18.0-Linux-x86_64/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release  -DPISTACHE_BUILD_EXAMPLES=false -DPISTACHE_BUILD_TESTS=false -DPISTACHE_BUILD_DOCS=false  -DPISTACHE_USE_SSL=false -DCMAKE_INSTALL_PREFIX=$PWD/../installpath ../
    make -j
    make install
    cp -r ../include/pistache /usr/local/include
    cp ./src/libpistache.a /usr/local/lib
    ls -l ../installpath/lib
    popd
    popd
}

build_prometheus() 
{
    set -xe 
    $SUDO apt-get install -y curl libcurl4-openssl-dev
    pushd /tmp
    wget $CMAKE_EXE 
    tar -zxvf cmake-3.18.0-Linux-x86_64.tar.gz
    $SUDO rm -rf /tmp/prometheus
    git clone -q $PROMETHEUS_CLIENT /tmp/prometheus
    pushd /tmp/prometheus
    git submodule init
    git submodule update
    mkdir -p _build && cd _build
    /tmp/cmake-3.18.0-Linux-x86_64/bin/cmake .. -DBUILD_SHARED_LIBS=ON && make -j 4 && $SUDO make install && $SUDO make DESTDIR=`pwd`/deploy install
    popd
    popd
}

(return 2>/dev/null) && echo "Sourced" && return
