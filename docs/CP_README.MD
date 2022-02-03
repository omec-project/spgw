License & Copyright
----

SPDX-License-Identifier: Apache-2.0

Copyright (c) 2017 Intel Corporation

SPGWC Control Plane
==================

#Low Level Code Walk-through
-----------------------------

## Initialization

* Master thread - Processes signaling messages, establishes PDN connections, and
  manages bearers.
* config thread - listens for config update triggers
* prometheus thread - listen the prometheus query and sends back the requested statistics
  to prometheus server.
* One thread for GTP incoming message read. Read the message from sockets and puts in the
  event queue for main thread
* One thread for GTP outgoing message. Take message one by one from the GTP outgoing queue
  and send the messasge out
* One thread for PFCP incoming message read. Read the message from sockets and puts in the
  event queue for main thread
* One thread for PFCP outgoing message. Take message one by one from the GTP outgoing queue
  and send the messasge out
* One thread for Gx incoming message read. Read the message from sockets and puts in the
  event queue for main thread
* One thread for Gx outgoing message. Take message one by one from the GTP outgoing queue
  and send the messasge out
* One thread for local thread message read. Read the message from sockets and puts in the
  event queue for main thread queue




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
