# License & Copyright
#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>

# SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

# UPF Failure
- UPF failure can be triggered with multiple events. Following is the list of
events which can trigger path failure,
1. No response to PFCP Heartbeat request
2. Received heartbeat request/response with different timestamp

- Currently Control Plane detects the failure with user plane by Method (1) i.e. no response from
user plane.

- Once Path failure is detected, control plane resolves the UPF endpoint again by resolving hostname
to ip address. THis helps control plane to point to new userplane ( in case user plane has changed its 
address)

