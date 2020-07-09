License & Copyright
----

SPDX-License-Identifier: Apache-2.0

SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

Copyright (c) 2017 Intel Corporation

NGIC Data Plane
===============

1. CDR flush to file.
---------------------
The API ue_cdr_flush(), flush the CDR of any session to a file.
This API accepts session id as key parameters and flush the CDR records.
The field cdr type can be set for specific type of records,
which can be one of CDR_TYPE_BEARER, CDR_TYPE_ADC, CDR_TYPE_FLOW,
CDR_TYPE_RG(rating group) or CDR_TYPE_ALL.
An action field can be set to append or replace the log file.
Please refer "struct msg_ue_cdr" for details.
Charging stats are exported without resetting CDRs in DP.
The file to which stats are dumped is located at "/var/log/dpn/session_cdr.csv".

9. Known Issues and limitations User Plane
-----------------------------
-       The current tested ADC rules are not fully conformant to 3GPP release 12 ADC AVPs.
-       ARP handling supported under KNI-Linux
-       ICMP supported under KNI-Linux
-       IPV4 fragmentation and option header not supported.
-       IPSec not supported.
-       Only ethernet v2 frame format is supported.
-       Logic issues found in prepare_acl_parameter() in dp/acl.c
-       ACL context's (struct rte_acl_ctx) trans_table parameter is not being checked
        for NULL by dpdk. trans_table may appear as NULL during handling of stray GTP
        packets by DP.
-       Updates to filter packets arriving at the S1U and SGI interfaces for
        processing by linux. ARP Packets OR
        packets w/ dest address = {[S1U_IP, SGI_IP]; MULTICAST ADDR; BROADCAST ADDR}
        are sent to Linux for processing. All other packets are sent to the UL or DL
        fast path cores for processing. These enhancements need to be tested for following,
                1- To be tested with targeted packets
                2- To be tested with S1U and SGI GW configuration
-       Addressed htonl byte order conversion issues, however the following items need to be addressed,
                1- Some inconsistencies continue to remain in these fields
                   @~/interface/interface.c::zmq_mbuf_process::CREATE_SESSION, MODIFY_SESSION,
                   DELETE_SESSION. These needs to be addressed over test scenarios/setups.
                2- Byte order changes to ~/cp/gtpv2c_messages/modify_bearer.c, delete_session.c need
                   to be thoroughly tested.
                3- This patch has NOT been tested for SGWC-SGWU, PGWC-PGWU, and dedicated bearer
                   configurations/test scenarios. Hence byte order changes to
                   ~/cp/gtpv2c_messages/create_bearer.c, create_s5s8_session.c,
                   delete_bearer.c, delete_s5s8_session.c have not been tested.
-		For running ngic-rtc without kni STATIC_ARP must be enabled in the Makefile.
		ether.c::construct_ether_hdr(...) needs linux (kni) support to ARP discover S1U, SGI peer MAC.

-		Without KNI option is only for preformance profiling. There is no support for:
		    - SGW-PGW separaton not suported in wokni option
		    - DP_DDN

10. Future Work
--------------
-	Limitations will be addressed
-	Performance optimization
-	IPV6 support
-	Neighbour Discovery and ICMPv6 stack support
-   A number of features are currently unsupported, such as unauthenticated IMSI device connection.
-   GTPv2c Piggybacking is unsupported

