#License & Copyright

#SPDX-FileCopyrightText: 2020 Open Networking Foundation <info@opennetworking.org>

#SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0



# procedures

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc|number of initial attach procedures started by SPGW|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|initial_attach_proc_result|UE initial attach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'INITIAL_ATTACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'sgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc|number of UE detach procedure started by spgw|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}]|[u'pgw_addr', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'pgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'MME_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|detach_ue_proc_result|UE detach procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_DETACH'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|s1_release_proc|number of S1 Release procedure started by SGW|[{u'cp_mode': u'sgw'}, {u'procedure': u'S1_RELEASE'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|s1_release_proc|number of S1 Release procedure started by SGW|[{u'cp_mode': u'spgw'}, {u'procedure': u'S1_RELEASE'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'S1_RELEASE'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'S1_RELEASE'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'S1_RELEASE'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|s1_release_proc_result|s1 release procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'S1_RELEASE'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc|number of service request procedure |[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|service_request_proc|number of service request procedure |[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'sgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|service_request_proc_result|service request procedure results|[{u'cp_mode': u'spgw'}, {u'procedure': u'SERVICE_REQUEST_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc|number of bearer create procedure |[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_create_proc|number of bearer create procedure |[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_create_proc_result|number of bearer create procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'DEDICATED_BEARER_ACTIVATION_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc|number of bearer update procedure |[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_update_proc|number of bearer update procedure |[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_update_proc_result|number of bearer update procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_UPDATE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc|number of bearer delete procedure |[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc|number of bearer delete procedure |[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|bearer_delete_proc_result|number of bearer delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'BEARER_DELETE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc|number of network initiated pdn delete procedure |[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc|number of network initiated pdn delete procedure |[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}]|[u'mme_addr', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|nw_init_pdn_delete_proc_result|number of network initiated pdn delete procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'NW_INIT_PDN_DELETE_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason', u'tac']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'sgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'mme_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'sgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'success'}]|[u'pgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'mme_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'sgw_addr', u'failure_reason']|
|Counter|number_of_procedures|gx_rar_proc_result|number of pcrf initiated RAR procedure|[{u'cp_mode': u'spgw'}, {u'procedure': u'RAR_PROC'}, {u'result': u'failure'}]|[u'pgw_addr', u'failure_reason']|


# data_usage

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Gauge|data_usage_of_subscribers|data_usage_of_subscribers|data usage of subscribers |[{u'cp_mode': u'spgw'}, {u'level': u'pdn'}]|[u'imsi']|


# num_ue

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'spgw'}, {u'state': u'active'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'spgw'}, {u'state': u'idle'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'pgw'}, {u'state': u'active'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'pgw'}, {u'state': u'idle'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'sgw'}, {u'state': u'active'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_subscribers|number of subscribers |[{u'cp_mode': u'sgw'}, {u'state': u'idle'}, {u'level': u'subscribers'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'spgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'spgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'spgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'spgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'pgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'pgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'pgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'pgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'sgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'sgw'}, {u'state': u'active'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'sgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'mme_addr', u'spgwu_addr']|
|Gauge|spgw_number_of_ue_attached|number_of_pdns|number of PDNs |[{u'cp_mode': u'sgw'}, {u'state': u'idle'}, {u'level': u'pdns'}]|[u'sgw_addr', u'spgwu_addr']|


# msg_tx

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'version_not_supported'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'version_not_supported'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'version_not_supported'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|version_not_supported|number of version not supported sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'version_not_supported'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CSRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CSRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'CSRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'CSRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'CSRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'CSRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'CSRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|create_session_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'CSRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'MBRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'MBRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'MBRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'MBRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'MBRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'MBRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'MBRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|modify_bearer_response|number of CSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'MBRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DSRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DSRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'DSRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'DSRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'DSRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'DSRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'DSRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|delete_session_response|number of DSRsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'DSRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'RABRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_rej': u'RABRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'RABRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|release_access_bearer_response|number of RAB Rsp sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_rej': u'RABRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_sent|downlink_data_notification_req|number of DDN Req sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DDNReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|create_bearer_req|number of Create Bearer Req sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CBReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|update_bearer_req|number of Update Bearer Req sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'UBReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|delete_bearer_req|number of Delete Bearer Req sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DBReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHOReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHOReq'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHOReq'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_request|number of gtp Echo Request sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHOReq'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHORsp'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHORsp'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHORsp'}]|[u'mme_addr']|
|Counter|number_of_messages_sent|gtpc_echo_response|number of gtp Echo Response sent by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHORsp'}]|[u'sgw_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_request|number of PFCP association sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_rej': u'AssocSetupRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_rej': u'AssocSetupRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_association_setup_response|number of PFCP association setup response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_rej': u'AssocSetupRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'PFDMGMTReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'PFDMGMTReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_pfd_management_req|number of PFCP PFD management reques sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'PFDMGMTReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_request|number of pfcp Echo Request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_echo_response|number of PFCP echo Response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessEstReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessEstReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_establishment_request|number of PFCP session est request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessEstReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessModReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessModReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_modification_request|number of PFCP session modification request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessModReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessDelReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessDelReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_delete_request|number of PFCP session delete request sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessDelReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessReportRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_rej': u'SessReportRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessReportRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_rej': u'SessReportRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessReportRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|pfcp_session_report_response|number of PFCP session report response sent by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_rej': u'SessReportRsp_rej'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCR_I'}]|[u'pcrf_addr']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCR_U'}]|[u'pcrf_addr']|
|Counter|number_of_messages_sent|gx_credit_control_request|number of CCR sent by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCR_T'}]|[u'pcrf_addr']|
|Counter|number_of_messages_sent|gx_reauthorization_answer|number of RAA sent by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'RAA'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_sent|gx_reauthorization_answer|number of RAA sent by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_rej': u'RAA_rej'}]|[u'pcrf_addr', u'reason']|


# msg_rx

|Family type|Family Name|Counter Name| Counter Help|Static Labels|Dynamic Labels|
|----------:|:--------------:|:------------:|:-----:|:-------:|-------:|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CSReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CSReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CSReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CSReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'CSReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'CSReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'CSReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_session_request|number of CSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'CSReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'MBReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'MBReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'MBReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'MBReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'MBReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'MBReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'MBReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|modify_bearer_request|number of MBReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'MBReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DSReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DSReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DSReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DSReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'DSReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'DSReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'DSReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_session_request|number of DSReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_drop': u'DSReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'RABReq'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'RABReq'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'RABReq_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|release_access_bearer_request|number of RABReq received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'RABReq_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DDNAck'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DDNAck'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DDNAck_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|downlink_data_notification|number of DDNAck received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DDNAck_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CBRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'CBRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CBRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CBRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CBRsp_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|create_bearer_response|number of create bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'CBRsp_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'UBRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'UBRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'UBRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'UBRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'UBRsp_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|update_bearer_response|number of update bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'UBRsp_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DBRsp'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'DBRsp'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DBRsp_rej'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DBRsp_rej'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DBRsp_drop'}]|[u'mme_addr', u'reason']|
|Counter|number_of_messages_received|delete_bearer_response|number of delete bearer response received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_drop': u'DBRsp_drop'}]|[u'sgw_addr', u'reason']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHOReq'}]|[u'mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHOReq'}]|[u'sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHOReq'}]|[u'mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_request|number of Echo Req received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHOReq'}]|[u'sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHORsp'}]|[u'mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's11'}, {u'msg_type': u'ECHORsp'}]|[u'sgw_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHORsp'}]|[u'mme_addr']|
|Counter|number_of_messages_received|gtpc_echo_response|number of Echo Rsp received by SPGW|[{u'protocol': u'gtpv2'}, {u'interface': u's5s8'}, {u'msg_type': u'ECHORsp'}]|[u'sgw_addr']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'AssocSetupReq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'AssocSetupReq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'AssocSetupReq'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_request|number of PFCP association setup request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'AssocSetupReq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'AssocSetupRsq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'AssocSetupRsq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'AssocSetupRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_association_setup_response|number of PFCP association setup response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'AssocSetupRsq_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'PFDMGMTRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'PFDMGMTRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'PFDMGMTRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'PFDMGMTRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'PFDMGMTRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_pfd_mgmt_response|number of PFCP PFD management response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'PFDMGMTRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_request|number of PFCP Echo Req received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'ECHOReq'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_echo_response|number of PFCP Echo Rsp received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'ECHORsp'}]|[u'spgwu_addr']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessEstRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'SessEstRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessEstRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'SessEstRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessEstRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_establishment_response|number of PFCP Sess Establishment received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'SessEstRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessModRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'SessModRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessModRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'SessModRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessModRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_modification_response|number of PFCP Sess Modification received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'SessModRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessDelRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'SessDelRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessDelRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'SessDelRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessDelRsp'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_delete_response|number of PFCP Session delete Response received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'SessDelRsp_drop'}]|[u'spgwu_addr', u'reason']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_type': u'SessReportReq'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_drop': u'SessReportReq_drop'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxa'}, {u'msg_rej': u'SessReportReq_rej'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_type': u'SessReportReq'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_drop': u'SessReportReq_drop'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'Sxb'}, {u'msg_rej': u'SessReportReq_rej'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_type': u'SessReportReq'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_drop': u'SessReportReq_drop'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|pfcp_session_report_request|number of PFCP session report request received by SPGW|[{u'protocol': u'pfcp'}, {u'interface': u'SxaSxb'}, {u'msg_rej': u'SessReportReq_rej'}]|[u'spgwu_addr', u'reason', u'report_type']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCA_I'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_drop': u'CCA_I_drop'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCA_U'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_drop': u'CCA_U_drop'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'CCA_T'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_credit_control_answer|number of CCR Answers received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_drop': u'CCA_T_drop'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_reauthorization_request|number of RAR received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_type': u'RAR'}]|[u'pcrf_addr', u'reason']|
|Counter|number_of_messages_received|gx_reauthorization_request|number of RAR received by SPGW|[{u'protocol': u'diameter'}, {u'interface': u'Gx'}, {u'msg_drop': u'RAR_drop'}]|[u'pcrf_addr', u'reason']|
