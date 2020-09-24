License & Copyright
----

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

# Message Collision handling at SPGW

Each time new request message is received by SPGW, a new procedure is created and
added into the user context queue. As of now only 1 procedure is processed per
session at any point in time. Once that procedure is finished/complete, new 
procedure is taken up from the queue. 

Tested following cases,
- DSReq to SPGW, while SPGW was processing previous MBReq. Current behavior is to
  queue DSReq. Once MBReq processing is complete then process queued DSReq.
