
License & Copyright
----

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>
#SPDX-License-Identifier: Apache-2.0

# Detach Procedure

# UE/MME/HSS Detach Procedure Testing

 - Successful handling of DSReq message and Sending back the Response to MME
 - DSReq validation failure (e.g. IEs not correct, Context not found for the GTP tunnel..)
 - Session delete timeout (PFCP Session delete failure,.. )
 - Retransmitted DSReq ( If retransmitted DSReq is received then just drop it and log message)

# PCRF/PDN-GW initiated detach

Sending out DBReq and handling Response of DBReq. Code exist but not tested and hence no test case listed here.
