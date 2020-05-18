# Multi-stage Dockerfile
ARG BASE_OS=ubuntu:16.04
ARG RUN_BASE=runtime

## Stage build: kitchen sink stage for compiling dependencies and CP/DP bins
FROM $BASE_OS as basepkg
ARG RTE_MACHINE=native

WORKDIR /ngic-rtc
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
FROM ossutil as ngic
RUN source ./build_ngic.sh && build_ngic

ENV LD_LIBRARY_PATH /ngic-rtc/libgtpv2c/lib:/ngic-rtc/libpfcp/lib:$LD_LIBRARY_PATH
FROM ngic as cp
COPY --from=ngic /ngic-rtc/cp/build/ngic_controlplane /bin/ngic_controlplane

FROM ngic as dp
COPY --from=ngic /ngic-rtc/dp/build/ngic_dataplane /bin/ngic_dataplane

#ajay - Need to cleanup image 
