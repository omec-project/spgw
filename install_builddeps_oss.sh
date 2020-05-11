#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Intel Corporation

SUDO=''
[[ $EUID -ne 0 ]] && SUDO=sudo

OSDIST=`lsb_release -is`
OSVER=`lsb_release -rs`


install_build_deps()
{
    $SUDO apt-get -y install g++ make cmake libuv-dev libssl-dev autotools-dev libtool-bin m4 automake libmemcached-dev memcached cmake-curses-gui gcc bison flex libsctp-dev libgnutls-dev libgcrypt-dev libidn11-dev nettle-dev
}

init_submodules()
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
  echo "step1 ***"
  virtualenv -p python3.5 venv
  echo "step2 ***"
  set +u
  source venv/bin/activate
  set -u
  echo "step3 ***"
  $SUDO pip install -r requirements.txt
  echo "step4 ***"
  deactivate
  echo "step5 ***"
  popd
}

build_c3po_util()
{
  make clean
  $SUDO make install
}
 
(return 2>/dev/null) && echo "Sourced" && return

set -o errexit
set -o pipefail
set -o nounset

install_build_deps
init_submodules
build_c3po_util

echo "Dependency install complete"

