#! /bin/bash

# SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
# Copyright (c) 2019 Sprint
#
# SPDX-License-Identifier: Apache-2.0

GX_APP_PATH="$PWD"
GX_APP="gx_app"
LOG_LEVEL=0

NOW=$(date +"%Y-%m-%d_%H-%M")
GX_FILE="$PWD/logs/gx_$NOW.log"

USAGE=$"Usage: run.sh"

if [ -z "$1" ]; then
	$GX_APP_PATH/$GX_APP

else
	echo "$USAGE"
fi
