License & Copyright
----

SPDX-License-Identifier: Apache-2.0 

SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

Copyright (c) 2017 Intel Corporation

SPGW-C
==============================================

1. Introduction
-------------------------------------------
The OMEC project is an open source LTE EPC development under Open Networking Foundation (ONF) consortium. 
Under OMEC, there are a suite of projects working together to create EPC solutions. The ‘spgwc’ project provides 
Gateway components of the EPC. This project provides sgw-c, pgw-c components. 

2.  Scope
----------
spgwc can be run on variety of servers running as a standalone application. It can run as POD in K8s deployment.

3.	Feature List
----------------
The SPGW-C currently supports the following SAE-GW features:
* All developement is based on 3gpp release 15 specifications
* CUPS compliant 
* Supports multiple SPGWU connectivity. 
* PFCP & GTP heartbeat support
* KPIs exported in prometheus.
* IPv4 support
* UPF selection based on DNS, local policy,
* Multiple APN Support
* Configuration update support
* Initial Attach support
* MME/HSS initiated detach support
* PFCP - session create,modify,delete
* S1 Release procedure
* Service Request procedure
* Dedicated bearer create/modify/delete
* Bearer Update 
* SGW relocation
* WIP - Gx interface. Testing in progress. CCR-I,CCR-U,CCR-T,RAR support
* Restoration procedure support
* GTPC, PFCP Echo support





4.	High Level Design
----------------------
Please read, more documentation under the docs directory. Those documents should give more details about the
supported features and design.

5.	Build, install, configure and test
------------------------------------------

Please refer to [install_notes](INSTALL.MD)



