License & Copyright
----

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#SPDX-License-Identifier: Apache-2.0


# Initial Attach Procedure

# Testing

    1. Attach Success
    2. CSReq validation failed (e.g. bad apn, no UE ip address, upf dns resolution fails.....)
    3. PFCP setup timeout....
    4. PFCP setup reject
    5. PFCP session setup reject
    6. PFCP session setup timeout
    7. Context Replacement while first CSReq was complete
    8. Context Replacement while first CSReq was in progress
    9. Retransmitted CSReq while first CSReq is still in progress. Just drop CSReq and log a message
    10. Attach request received, but subscriber classification config is not yet available.
      So leads to reject CSReq
    11. Second UE attach while PFCP session in progress 
    12. Second UE attach while PFCP session is already established 
   
# WIP
- Enable gx config and setup initial call with PCRF. Some of the AVPs are not 
  not correct. e.g. bearer identitier is sent as 35 instead of 5.
- Graceful cleanup when context replacement. 

# Special cases which needs more investigation
    1. setup response has different nodeid or FQDN. 
