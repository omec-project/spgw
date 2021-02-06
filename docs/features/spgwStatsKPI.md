#License & Copyright

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>

#SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0



# num_ue

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'spgw'}, {'state': 'active'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'spgw'}, {'state': 'idle'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'pgw'}, {'state': 'active'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'pgw'}, {'state': 'idle'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'sgw'}, {'state': 'active'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{'cp_mode': 'sgw'}, {'state': 'idle'}, {'level': 'subscribers'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'spgw'}, {'state': 'active'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'spgw'}, {'state': 'active'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'spgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'spgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'pgw'}, {'state': 'active'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'pgw'}, {'state': 'active'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'pgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'pgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'sgw'}, {'state': 'active'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'sgw'}, {'state': 'active'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'sgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['mme_addr', 'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{'cp_mode': 'sgw'}, {'state': 'idle'}, {'level': 'pdns'}]|['sgw_addr', 'spgwu_addr']|


# msg_rx

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CSReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CSReq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CSReq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CSReq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'CSReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'CSReq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'CSReq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'CSReq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'mbreq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'mbreq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'mbreq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'mbreq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'mbreq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'mbreq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'mbreq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'mbreq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DSReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DSReq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'dsreq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'dsreq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'DSReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'DSReq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'dsreq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_drop': 'dsreq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'RABReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'RABReq'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'RABReq_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'RABReq_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'RABReq_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'RABReq_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DDNAck'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DDNAck'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DDNAck_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DDNAck_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CBRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CBRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CBRsp_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CBRsp_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CBRsp_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'CBRsp_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'UBRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'UBRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'UBRsp_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'UBRsp_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'UBRsp_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'UBRsp_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DBRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DBRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DBRsp_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DBRsp_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DBRsp_drop'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_drop': 'DBRsp_drop'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHOReq'}]|['mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHOReq'}]|['sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHOReq'}]|['mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHOReq'}]|['sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHORsp'}]|['mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHORsp'}]|['sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHORsp'}]|['mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHORsp'}]|['sgw_addr']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'AssocSetupReq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'AssocSetupReq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'AssocSetupReq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'AssocSetupRsq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'AssocSetupRsq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'AssocSetupRsq_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'PFDMGMTRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'PFDMGMTRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'PFDMGMTRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'PFDMGMTRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'PFDMGMTRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'PFDMGMTRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessEstRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'SessEstRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessEstRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'SessEstRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessEstRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'SessEstRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessModRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'SessModRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessModRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'SessModRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessModRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'SessModRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessDelRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'SessDelRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessDelRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'SessDelRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessDelRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'SessDelRsp_drop'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessReportReq'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_drop': 'SessReportReq_drop'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_rej': 'SessReportReq_rej'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessReportReq'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_drop': 'SessReportReq_drop'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_rej': 'SessReportReq_rej'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessReportReq'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_drop': 'SessReportReq_drop'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_rej': 'SessReportReq_rej'}]|['spgwu_addr', 'reason', 'report_type']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCA_I'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_drop': 'CCA_I_DROP'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCA_U'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_drop': 'CCA_U_DROP'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCA_T'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_drop': 'CCA_T_DROP'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_reauthorization_request|number of RAR received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'RAR'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_received|gx_reauthorization_request|number of RAR received by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_drop': 'RAR_DROP'}]|['pcrf_addr', 'reason']|


# msg_tx

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'version_not_supported'}]|['mme_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'version_not_supported'}]|['sgw_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'version_not_supported'}]|['mme_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'version_not_supported'}]|['sgw_addr']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CSRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CSRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'CSRsp_REJ'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'CSRsp_REJ'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'CSRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'CSRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'CSRsp_REJ'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'CSRsp_REJ'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'MBRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'MBRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'MBRsp_REJ'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'MBRsp_REJ'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'MBRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'MBRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'MBRsp_REJ'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'MBRsp_REJ'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DSRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DSRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'DSRsp_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'DSRsp_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'DSRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'DSRsp'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'DSRsp_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'DSRsp_rej'}]|['sgw_addr', 'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'RABRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_rej': 'RABRSP_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'RABRsp'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_rej': 'RABRSP_rej'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|downlink_data_notification_req|number of DDN Req sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DDNReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|create_bearer_req|number of Create Bearer Req sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'CBReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|update_bearer_req|number of Update Bearer Req sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'UBReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|delete_bearer_req|number of Delete Bearer Req sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'DBReq'}]|['mme_addr', 'reason']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHOReq'}]|['mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHOReq'}]|['sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHOReq'}]|['mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHOReq'}]|['sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHORsp'}]|['mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's11'}, {'msg_type': 'ECHORsp'}]|['sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHORsp'}]|['mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{'protocol': 'gtpv2'}, {'interface': 's5s8'}, {'msg_type': 'ECHORsp'}]|['sgw_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'AssocSetupReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_rej': 'AssocSetupRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_rej': 'AssocSetupRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'AssocSetupRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_rej': 'AssocSetupRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'PFDMGMTReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'PFDMGMTReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'PFDMGMTReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'ECHOReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'ECHORsp'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessEstReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessEstReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessEstReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessModReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessModReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessModReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessDelReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessDelReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessDelReq'}]|['spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_type': 'SessReportRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxa'}, {'msg_rej': 'SessReportRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_type': 'SessReportRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'Sxb'}, {'msg_rej': 'SessReportRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_type': 'SessReportRsp'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{'protocol': 'pfcp'}, {'interface': 'SxaSxb'}, {'msg_rej': 'SessReportRsp_rej'}]|['spgwu_addr', 'reason']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCR_I'}]|['pcrf_addr']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCR_U'}]|['pcrf_addr']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'CCR_T'}]|['pcrf_addr']|
|Counter|number_of_messages_sent|gx_reauthorization_answer|number of RAA sent by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_type': 'RAA'}]|['pcrf_addr', 'reason']|
|Counter|number_of_messages_sent|gx_reauthorization_answer|number of RAA sent by SPGW|[{'protocol': 'diameter'}, {'interface': 'Gx'}, {'msg_rej': 'RAA_rej'}]|['pcrf_addr', 'reason']|


# procedures

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'sgw'}, {'procedure': 'INITIAL_ATTACH'}]|['mme_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'sgw'}, {'procedure': 'INITIAL_ATTACH'}]|['sgw_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'sgw'}, {'procedure': 'INITIAL_ATTACH'}]|['pgw_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'pgw'}, {'procedure': 'INITIAL_ATTACH'}]|['mme_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'pgw'}, {'procedure': 'INITIAL_ATTACH'}]|['sgw_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'pgw'}, {'procedure': 'INITIAL_ATTACH'}]|['pgw_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'spgw'}, {'procedure': 'INITIAL_ATTACH'}]|['mme_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'spgw'}, {'procedure': 'INITIAL_ATTACH'}]|['sgw_addr']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{'cp_mode': 'spgw'}, {'procedure': 'INITIAL_ATTACH'}]|['pgw_addr']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'sgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'sgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'sgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'sgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'pgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'pgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'pgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'pgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'spgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'spgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'spgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{'cp_mode': 'spgw'}, {'proc_type': 'INITIAL_ATTACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'mme_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'mme_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'mme_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'nw_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'nw_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'sgw'}, {'procedure': 'nw_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'mme_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'mme_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'mme_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'nw_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'nw_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'pgw'}, {'procedure': 'nw_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'mme_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'mme_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'mme_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'nw_init_detach'}]|['mme_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'nw_init_detach'}]|['sgw_addr']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{'cp_mode': 'spgw'}, {'procedure': 'nw_init_detach'}]|['pgw_addr']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'pgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'MME_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_DETACH'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|s1_release_proc|number of S1 Release procedure started by SGW|[{'cp_mode': 'sgw'}, {'procedure': 'S1_RELEASE'}]|['mme_addr']|
|Counter|number_of_procedures|s1_release_proc|number of S1 Release procedure started by SGW|[{'cp_mode': 'spgw'}, {'procedure': 'S1_RELEASE'}]|['mme_addr']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'S1_RELEASE'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'S1_RELEASE'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'S1_RELEASE'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'S1_RELEASE'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc|number of service request procedure |[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|service_request_proc|number of service request procedure |[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'sgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{'cp_mode': 'spgw'}, {'procedure': 'SERVICE_REQUEST_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc|number of bearer create procedure |[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_create_proc|number of bearer create procedure |[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'sgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{'cp_mode': 'spgw'}, {'procedure': 'DEDICATED_BEARER_ACTIVATION_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc|number of bearer update procedure |[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_update_proc|number of bearer update procedure |[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_UPDATE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc|number of bearer delete procedure |[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_delete_proc|number of bearer delete procedure |[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'BEARER_DELETE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc|number of network initiated pdn delete procedure |[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc|number of network initiated pdn delete procedure |[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'sgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{'cp_mode': 'spgw'}, {'procedure': 'NW_INIT_PDN_DELETE_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc|number of pcrf initiated RAR procedure |[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|gx_rar_proc|number of pcrf initiated RAR procedure |[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}]|['mme_addr']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'sgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'success'}]|['pgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['mme_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['sgw_addr', 'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{'cp_mode': 'spgw'}, {'procedure': 'RAR_PROC'}, {'result': 'failure'}]|['pgw_addr', 'failure_reason']|


# data_usage

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Gauge|data_usage_of_subscribers|data_usage_of_subscribers|data usage of subscribers |[{'cp_mode': 'spgw'}, {'level': 'pdn'}]|['imsi']|
