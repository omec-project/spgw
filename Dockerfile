# Multi-stage Dockerfile
ARG BASE_OS=ubuntu:16.04
ARG RUN_BASE=runtime

## Stage build: kitchen sink stage for compiling dependencies and CP/DP bins
FROM $BASE_OS as basepkg
ARG RTE_MACHINE=native

WORKDIR /ngic-rtc
SHELL ["/bin/bash", "-c"]

COPY git_url.cfg . 
COPY docker-scripts ./docker-scripts
COPY patches ./patches
COPY dpdk-18.02_common_linuxapp .

RUN source ./docker-scripts/install_pkg_deps.sh && install_pkg_deps

FROM basepkg as dpdkbuild
RUN source ./docker-scripts/install_dpdk.sh && install_dpdk

FROM dpdkbuild as freediambuild
RUN source ./docker-scripts/install_freediameter.sh && download_freediameter 

FROM freediambuild as hyperscanbuild
RUN source ./docker-scripts/install_hyperscan.sh && install_hyperscan 

FROM hyperscanbuild as ossutil
RUN source ./docker-scripts/install_oss_util.sh && install_oss_util 

COPY . ./
