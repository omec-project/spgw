# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

# Multi-stage Dockerfile
ARG BASE_OS=ubuntu:16.04

FROM $BASE_OS as basepkg
WORKDIR /spgw
SHELL ["/bin/bash", "-c"]

COPY git_url.cfg . 
COPY patches ./patches
COPY docker-scripts ./docker-scripts/
RUN source ./docker-scripts/install_builddeps.sh && \
    install_builddeps

FROM basepkg as webutils
RUN source ./docker-scripts/install_builddeps.sh && \
    source ./docker-scripts/install_webutils.sh && \
    build_prometheus && build_pistache 

FROM webutils as spgw
COPY . ./
ARG CPUS
ARG RTE_MACHINE=native
ARG EXTRA_CFLAGS='-ggdb -O2'
RUN source ./docker-scripts/install_builddeps.sh && \
    source ./docker-scripts/build_spgw.sh && \
    build_spgw

FROM $BASE_OS as runtime
SHELL ["/bin/bash", "-c"]
COPY ./docker-scripts/install_rundeps.sh .
RUN source ./install_rundeps.sh && install_run_cp_deps && cleanup_image

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH
FROM runtime as cp
COPY --from=spgw /usr/local/lib /usr/local/lib
COPY --from=spgw /spgw/cp/bin/ngic_controlplane /bin/ngic_controlplane
COPY --from=spgw /spgw/libgtpv2c/lib/libgtpv2c.so /usr/local/lib
COPY --from=spgw /spgw/libpfcp/lib/libpfcp.so /usr/local/lib
COPY --from=spgw /spgw/third_party/freediameter/build/libfdcore/libfdcore.so /usr/local/lib
COPY --from=spgw /spgw/third_party/freediameter/build/libfdproto/libfdproto.so /usr/local/lib
COPY --from=spgw /spgw/cpplib/target/lib/libspgwcpputil.a /usr/local/lib/
COPY --from=spgw /tmp/prometheus/_build/deploy/usr/local/lib/ /usr/local/lib/
COPY --from=spgw /spgw/cp/gx/gx_app/bin/gx_app  /bin/
COPY --from=spgw /spgw/cp/gx/gx_app/bin/make_certs.sh  /bin/
COPY --from=spgw /tmp/pistache/installpath/lib/lib* /usr/local/lib/
