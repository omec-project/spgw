License & Copyright
----

SPDX-License-Identifier: Apache-2.0 

SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

Copyright (c) 2017 Intel Corporation

Next Generation Infrastructure Core (NGIC) VNF
==============================================

1. Introduction
-------------------------------------------
Evolved Packet Core (EPC) is a critical node in the wireless infrastructure,
which provides the data services to the end users. The NGIC is a
virtualized application providing the same service as the standalone node.
The SAE-GW (S-GW and P-GW)is the user plane node for the EPC which deals with
converged voice and data services on Long Term Evolution (LTE) networks. This project
implements SPGWC. 

2.  Scope
----------
NGIC can be run on variety of servers running as a standalone application. 

3.	Feature List
----------------
The NGIC VNF currently supports the following SAE-GW features:
* CUPS compliant 
* Supports multiple SPGWU connectivity. 
* PFCP & GTP heartbeat support
* Enable command line or display stats periodically.
* IPv4 support

4.	High Level Design
----------------------
Please refer to Figure 1 for the basic blocks in the NGIC VNG.

```text
        +----------------+
        |                |
+-----> |    Control     |---------->
 S11    |    Plane       | Gx
<-----+ |                |<---------
        |                |
        +-------+--------+
                |
                |
                | PFCP
                |
                v
```

5.	Build, install, configure and test
------------------------------------------

Please refer to [install_notes] (INSTALL.MD)

6.	Test Plan
------------------

