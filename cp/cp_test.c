// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <rte_common.h>
#include "cp_test.h"
#include "cp_events.h"
#include "clogger.h"
#include "cp_io_poll.h" 
#include "gx_interface.h"
#include "util.h"
#include "gtp_messages_decoder.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_session.h"
#include "gtp_ies.h"

test_out_gtp_handler gtp_mock_handler[256];

void* 
test_event_thread(void* data)
{
    RTE_SET_USED(data);
    printf("Starting test event handler thread \n");
    init_gtp_mock_interface();
    stack_event_t *event;
    while(1) {
        event  = (stack_event_t *)get_test_stack_unwind_event();
        if(event != NULL)
        {
            clLog(clSystemLog, eCLSeverityCritical,"handle stack unwind event %s ",event_names[event->event]);
            event->cb(event->data, event->event);
            free(event);
            continue;
        }
        usleep(10);
    }
    printf("exiting event handler thread \n");
    return NULL;
}

void 
test_event_handler(void *data, uint16_t evt_id)
{
    if(evt_id == TEST_EVENTS) {
        sleep(30);
        pdn_connection_t *pdn_ctxt = (pdn_connection_t *)data; 
        gx_context_t *gx_ctxt = pdn_ctxt->context->gx_context;

        msg_info_t *msg = calloc(1, sizeof(msg_info_t)); 
        msg->msg_type = GX_RAR_MSG;
        msg->source_interface = GX_IFACE;
	    gx_msg rar_request = {0};
		uint16_t msglen = 0;
		char *rar_buffer = NULL;
        printf("Received Test events for session Id = %s\n",gx_ctxt->gx_sess_id);
        rar_request.data.cp_rar.presence.session_id = PRESENT;
        rar_request.data.cp_rar.session_id.len = strlen(gx_ctxt->gx_sess_id);
        memcpy(rar_request.data.cp_rar.session_id.val, gx_ctxt->gx_sess_id, strlen(gx_ctxt->gx_sess_id));
        rar_request.data.cp_rar.presence.charging_rule_install = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.count = 1;
        rar_request.data.cp_rar.charging_rule_install.list = calloc(1, sizeof(GxChargingRuleInstall));
        rar_request.data.cp_rar.charging_rule_install.list[0].presence.charging_rule_name = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.count  = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.list  = calloc(1, sizeof(GxChargingRuleNameOctetString));
        strcpy((char *)rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.list[0].val,"PRESENT");
        rar_request.data.cp_rar.charging_rule_install.list[0].presence.charging_rule_definition = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.count = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list = calloc(1, sizeof(GxChargingRuleDefinition));
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].presence.charging_rule_name = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].charging_rule_name.len = strlen("rule2");
        strcpy((char *)rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].charging_rule_name.val,"rule2");

        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].presence.qos_information = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.qos_class_identifier = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.qos_class_identifier = 5;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.max_requested_bandwidth_dl = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.max_requested_bandwidth_dl = 222222;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.max_requested_bandwidth_ul = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.max_requested_bandwidth_ul = 222223;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.guaranteed_bitrate_dl = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.guaranteed_bitrate_dl = 1111111;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.guaranteed_bitrate_ul = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.guaranteed_bitrate_ul = 1111112;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.presence.allocation_retention_priority = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.presence.priority_level = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.priority_level = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.presence.pre_emption_capability= PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.pre_emption_capability = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.presence.pre_emption_vulnerability= PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].qos_information.allocation_retention_priority.pre_emption_vulnerability= 1;

        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].presence.flow_information = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.count =  1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list =  calloc(1,sizeof(GxFlowInformation));
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].presence.flow_description =  PRESENT;
        char rule[300]="permit out udp from 0.0.0.0/0 0-65535 to 0.0.0.0/0 0-65535";
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_description.len =strlen(rule);
        strcpy((char *)rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_description.val, rule);

        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].presence.flow_direction = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_direction = BIDIRECTIONAL;

        msglen = gx_rar_calc_length(&rar_request.data.cp_rar);
        rar_buffer = calloc(1, msglen+sizeof(rar_request.msg_type)+4);
	    memcpy(rar_buffer, &rar_request.msg_type, sizeof(rar_request.msg_type));
	    gx_rar_pack(&(rar_request.data.cp_rar), (unsigned char *)(rar_buffer + sizeof(rar_request.msg_type) + sizeof(rar_request.seq_num)), msglen); 
        msg->raw_buf = rar_buffer;
        fflush(NULL);
        queue_stack_unwind_event(GX_MSG_RECEIVED, (void *)msg, process_gx_msg);
    }
    return ;
}

void 
init_gtp_mock_interface(void)
{
    for(int i=0;i<256;i++)
        gtp_mock_handler[i] = NULL;

    // enable this hook to feed outgoing messages to custom handler 
    gtp_mock_handler[GTP_CREATE_BEARER_REQ] =  handle_mock_create_bearer_request_msg;
}

void 
handle_unknown_mock_msg(void *event)
{
    printf("Received unknown mock msg event %p\n",event);
    return;
}

void 
handle_mock_create_bearer_request_msg(void *evt)
{
    int ret;
    RTE_SET_USED(ret);
    msg_info_t *msg = calloc(1, sizeof(msg_info_t));
    outgoing_pkts_event_t *event = (outgoing_pkts_event_t *)evt;

    printf("%s Received mock event %p\n",__FUNCTION__,event);

    if((ret = decode_create_bearer_req((uint8_t *) event->payload,
                    &msg->gtpc_msg.cb_req) == 0))
        return;

    uint8_t  cbrsp_pkt[1000];
    uint16_t cbrsp_len;
    {
        gtp_create_bearer_request_bearer_ctxt_ie_t *cbreq_bc = &msg->gtpc_msg.cb_req.bearer_contexts;
        create_bearer_rsp_t cbrsp = {0}; 
        cbrsp.header.gtpc.teid_flag =  1;
        cbrsp.header.gtpc.version = 2; 
        cbrsp.header.gtpc.message_type = GTP_CREATE_BEARER_RSP; 
        cbrsp.header.gtpc.message_len = 10; 
        cbrsp.header.teid.has_teid.teid = 0; 
        cbrsp.header.teid.has_teid.seq = msg->gtpc_msg.cb_req.header.teid.has_teid.seq; 

        set_cause_accepted(&cbrsp.cause, 0); 

        set_ebi(&cbrsp.bearer_contexts.eps_bearer_id, 0, 6);
        set_cause_accepted(&cbrsp.bearer_contexts.cause, 0);
        struct in_addr en_addr; 
        en_addr.s_addr = 0x01010101;
        set_ipv4_fteid(&cbrsp.bearer_contexts.s1u_enb_fteid, GTPV2C_IFTYPE_S1U_ENODEB_GTPU, 0, en_addr, 100); 

        struct in_addr sgw_addr; 
        sgw_addr.s_addr = cbreq_bc->s1u_sgw_fteid.ipv4_address;
        uint32_t teid = cbreq_bc->s1u_sgw_fteid.teid_gre_key;
        set_ipv4_fteid(&cbrsp.bearer_contexts.s1u_sgw_fteid, GTPV2C_IFTYPE_S1U_SGW_GTPU, 0, sgw_addr, teid);
        set_ie_header(&cbrsp.bearer_contexts.header, 
                      GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO,
			          (cbrsp.bearer_contexts.eps_bearer_id.header.len
			           + sizeof(ie_header_t)
			           + cbrsp.bearer_contexts.cause.header.len
			           + sizeof(ie_header_t)
			           + cbrsp.bearer_contexts.s1u_enb_fteid.header.len
			           + sizeof(ie_header_t)
			           + cbrsp.bearer_contexts.s1u_sgw_fteid.header.len
			           + sizeof(ie_header_t)));                      
        cbrsp_len = encode_create_bearer_rsp(&cbrsp, cbrsp_pkt);
        printf("\n cbrsp len = %d \n", cbrsp_len);
        cbrsp.header.gtpc.message_len = htons(cbrsp_len);
        printf("\n cbrsp len = %d \n", cbrsp.header.gtpc.message_len);
        gtpv2c_header_t *cbrsp_header = (gtpv2c_header_t *)cbrsp_pkt;
        cbrsp_header->gtpc.message_len = htons(cbrsp_len);
    }


    {
        struct sockaddr_in peer_sockaddr = {0};
        msg_info_t *cbrsp_event = calloc(1, sizeof(msg_info_t));
        cbrsp_event->peer_addr = peer_sockaddr;
        cbrsp_event->source_interface = S11_IFACE;
        cbrsp_event->msg_type = GTP_CREATE_BEARER_RSP;
        cbrsp_event->raw_buf = calloc(1, event->payload_len);
        memcpy(cbrsp_event->raw_buf, cbrsp_pkt, cbrsp_len);
        queue_stack_unwind_event(GTP_MSG_RECEIVED, (void *)cbrsp_event, process_gtp_msg);
    }
    return ;
}
