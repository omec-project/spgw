#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Intel Corporation

SUDO=''
[[ $EUID -ne 0 ]] && SUDO=sudo

install_run_cp_deps() {
	$SUDO apt-get update && $SUDO apt-get -y install \
		libnuma1 \
		openssl \
		libidn11 \
		libgnutls30 \
		libsctp1 \
		netbase\
		openssh-server\
		sshpass \
		libpcap0.8\
        gdb

}

install_run_utils() {
	$SUDO apt-get update && $SUDO apt-get -y install \
		dnsutils \
		iproute2 \
		iputils-ping \
		tcpdump
}

cleanup_image() {
	$SUDO rm -rf /var/lib/apt/lists/*
}

(return 2>/dev/null) && echo "Sourced" && return
