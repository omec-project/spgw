License & Copyright
----

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#SPDX-License-Identifier: Apache-2.0

## Goal 
- For each call cp-mode is set depending on SGW/PGW selection done by MME.
- Call can move from saegw mode to PGW mode or PGW mode to SAEGW mode.
- cp_mode configuration will be used to figure out which sockets should be opened. Refer cp.cfg in the config  

## WIP
- There are few things which needs to be tested to make sure it works consistently.

## Note - Most of the cases are by default tested for SAEGW mode.
