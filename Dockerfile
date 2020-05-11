# Multi-stage Dockerfile
ARG BASE_OS=ubuntu:16.04
ARG RUN_BASE=runtime

## Stage build: kitchen sink stage for compiling dependencies and CP/DP bins
FROM $BASE_OS as pfcpbuild
ARG CPUS
ARG RTE_MACHINE=native
ARG EXTRA_CFLAGS='-DUSE_AF_PACKET -ggdb -O2'

WORKDIR /ngic-rtc
SHELL ["/bin/bash", "-c"]

COPY . ./
RUN ./install_builddeps.sh

RUN ./build_ngic.sh
