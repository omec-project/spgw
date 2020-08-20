#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

build_c_ares()
{
  pushd modules/c-ares
  ./buildconf
  ./configure
  make
  $SUDO make install
  popd
  
  BUILD_C_ARES_COMPLETE="- COMPLETE"
}

build_cpp_driver()
{
  pushd modules/cpp-driver
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_CPP_DRIVER_COMPLETE="- COMPLETE"
}

build_pistache()
{
  pushd modules/pistache
  rm -rf build
  mkdir -p build
  cd build
  cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
  make
  $SUDO make install
  popd
  
  BUILD_PISTACHE_COMPLETE="- COMPLETE"
}

build_rapidjson()
{
  pushd modules/rapidjson
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_RAPIDJSON_COMPLETE="- COMPLETE"
}

build_spdlog()
{
  pushd modules/spdlog
  rm -rf build
  mkdir -p build
  cd build
  cmake ..
  make
  $SUDO make install
  popd
  
  BUILD_SPDLOG_COMPLETE="- COMPLETE"
}

build_cli()
{
  pushd cli
  $SUDO apt-get -y install python-pip
  $SUDO pip install -r requirements.txt
  $SUDO apt-get -y install python-virtualenv
  virtualenv -p python3.5 venv
  set +u
  source venv/bin/activate
  set -u
  $SUDO pip install -r requirements.txt
  #ajay : should i check with GSlab ??
  #deactivate
  popd
}

init_oss_util_submodules()
{
  git submodule init
  git submodule update

  build_c_ares
  build_cpp_driver
  build_pistache
  build_rapidjson
  build_spdlog
  build_cli 
 
  $SUDO ldconfig
}

build_c3po_util()
{
  make clean
  make
  $SUDO make install
}

install_oss_util()
{
   mkdir -p $SPGW_DIR/$C3PO_OSS_DIR
   pushd $SPGW_DIR/$C3PO_OSS_DIR
   git clone $OSS_UTIL_GIT_LINK
   pushd oss-util
   init_oss_util_submodules
   #build_c3po_util
   popd
   popd
}

(return 2>/dev/null) && echo "Sourced" && return
