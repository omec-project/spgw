License & Copyright
----

SPDX-License-Identifier: Apache-2.0

SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

Copyright (c) 2017 Intel Corporation

SPGWC Control Plane
==================

#Low Level Code Walk-through
-----------------------------

## Initialization

DPDK EAL arguments are parsed first. Please refer to
[DPDK Environment Abstraction Layer documentation](
http://dpdk.org/doc/guides/prog_guide/env_abstraction_layer.html).
The parsing of Control Plane specific arguments follows. Both EAL and Control
Plane arguments are described in the installation documentation [here](INSTALL.MD).

* Master thread - Processes signaling messages, establishes PDN connections, and
  manages bearers.
* config thread - listens for config update triggers
* prometheus thread - listen the prometheus query and sends back the requested statistics
  to prometheus server.

## Master Thread Operation

The Master Thread processes signaling messages, which may be broken down by 3GPP
Procedure Events as described by 3GPP TS 23.401. 

## Error Detection and Handling
- if any request message is not valid then message is validated and if message is found
  to be invalid/wrong then reject message is generated.
- Retransmission of the messages is detected on PFCP and GTPv2 interface
- In case SPGWC has initiated request and no response then messages are retransmitted.
- In case no response even after number of retransmission attempts then message timeout
  handling is present.
