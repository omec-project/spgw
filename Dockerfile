# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only

# Multi-stage Dockerfile
ARG BASE_OS=ubuntu:16.04

## Stage build: kitchen sink stage for compiling dependencies and CP/DP bins
FROM $BASE_OS as basepkg
ARG RTE_MACHINE=native

WORKDIR /spgw
SHELL ["/bin/bash", "-c"]

COPY git_url.cfg . 
COPY patches ./patches
COPY dpdk-18.02_common_linuxapp .

COPY docker-scripts/install_pkg_deps.sh ./docker-scripts/
RUN source ./docker-scripts/install_pkg_deps.sh && install_pkg_deps

FROM basepkg as dpdkbuild
COPY docker-scripts/install_dpdk.sh ./docker-scripts/
RUN source ./docker-scripts/install_dpdk.sh && install_dpdk

FROM dpdkbuild as freediambuild
COPY docker-scripts/install_freediameter.sh ./docker-scripts/
RUN source ./docker-scripts/install_freediameter.sh && download_freediameter 

FROM freediambuild as hyperscanbuild
COPY docker-scripts/install_hyperscan.sh ./docker-scripts/
RUN source ./docker-scripts/install_hyperscan.sh && install_hyperscan 

FROM hyperscanbuild as ossutil
COPY docker-scripts/install_oss_util.sh ./docker-scripts/
RUN source ./docker-scripts/install_oss_util.sh && install_oss_util 

COPY . ./
ARG EXTRA_CFLAGS='-DUSE_AF_PACKET -ggdb -O2'
FROM ossutil as spgw
RUN source ./docker-scripts/build_spgw.sh && build_spgw

## Stage runtime: no utils
FROM $BASE_OS as runtime
SHELL ["/bin/bash", "-c"]
COPY ./docker-scripts/install_rundeps.sh .

## Stage cp: creates the runtime image of control plane
ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH
FROM runtime as cp
RUN source ./install_rundeps.sh && install_run_cp_deps && cleanup_image
COPY --from=spgw /spgw/cp/build/ngic_controlplane /bin/ngic_controlplane
COPY --from=spgw /spgw/libgtpv2c/lib/libgtpv2c.so /usr/local/lib
COPY --from=spgw /spgw/libpfcp/lib/libpfcp.so /usr/local/lib
COPY --from=spgw /spgw/third_party/freediameter/build/libfdcore/libfdcore.so /usr/local/lib
COPY --from=spgw /spgw/third_party/freediameter/build/libfdproto/libfdproto.so /usr/local/lib
COPY --from=spgw /spgw/oss_adapter/c3po_oss/oss-util/modules/cpp-driver/build/libcassandra.so /usr/local/lib/
COPY --from=spgw /spgw/oss_adapter/c3po_oss/oss-util/modules/c-ares/.libs/libcares.so /usr/local/lib/

FROM runtime as dp
RUN source ./install_rundeps.sh && install_run_utils && cleanup_image
COPY --from=spgw /spgw/dp/build/ngic_dataplane /bin/ngic_dataplane
COPY --from=spgw /spgw/libpfcp/lib/libpfcp.so /usr/local/lib

#ajay - Need to cleanup image 
