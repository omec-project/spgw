// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include "cp_test.h"
#include "cp_events.h"
#include "cp_io_poll.h" 
#include "gx_interface.h"
#include "util.h"
#include "gtp_messages_decoder.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_session.h"
#include "gtp_ies.h"
#include "cp_log.h"
#include "pdn.h"

test_out_pkt_handler gtp_out_mock_handler[256];
test_out_pkt_handler pfcp_out_mock_handler[256];
test_out_pkt_handler gx_out_mock_handler[256];

test_in_pkt_handler gtp_in_mock_handler[256];
test_in_pkt_handler pfcp_in_mock_handler[256];
test_in_pkt_handler gx_in_mock_handler[256];


void* 
test_event_thread(void* data)
{
    LOG_MSG(LOG_INIT, "Starting test event handler thread ");
	stack_event_t *event;
    while(1) {
        event  = (stack_event_t *)get_test_stack_unwind_event();
        if(event != NULL)
        {
            //LOG_MSG(LOG_INFO,"handle stack unwind event %s ",event_names[event->event]);
            event->cb(event->data, event->event);
            free(event);
        }
        usleep(100); // every pkt 0.1 ms default scheduling delay
    }
    LOG_MSG(LOG_ERROR, "exiting event handler thread %p", data);
    return NULL;
}

void 
test_event_handler(void *data, uint16_t evt_id)
{
    if(evt_id == TEST_EVENTS) {
        sleep(30);
        pdn_connection_t *pdn_ctxt = (pdn_connection_t *)data; 
        gx_context_t *gx_ctxt = (gx_context_t *)pdn_ctxt->context->gx_context;

        msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t)); 
        msg->msg_type = GX_RAR_MSG;
        msg->source_interface = GX_IFACE;
	    gx_msg rar_request = {0};
		uint16_t msglen = 0;
		char *rar_buffer = NULL;
        LOG_MSG(LOG_INFO, "Received Test events for session Id = %s",gx_ctxt->gx_sess_id);
        rar_request.data.cp_rar.presence.session_id = PRESENT;
        rar_request.data.cp_rar.session_id.len = strlen(gx_ctxt->gx_sess_id);
        memcpy(rar_request.data.cp_rar.session_id.val, gx_ctxt->gx_sess_id, strlen(gx_ctxt->gx_sess_id));
        rar_request.data.cp_rar.presence.charging_rule_install = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.count = 1;
        rar_request.data.cp_rar.charging_rule_install.list = (GxChargingRuleInstall *)calloc(1, sizeof(GxChargingRuleInstall));
        rar_request.data.cp_rar.charging_rule_install.list[0].presence.charging_rule_name = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.count  = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.list  = (GxChargingRuleNameOctetString *)calloc(1, sizeof(GxChargingRuleNameOctetString));
        strcpy((char *)rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_name.list[0].val,"PRESENT");
        rar_request.data.cp_rar.charging_rule_install.list[0].presence.charging_rule_definition = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.count = 1;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list = (GxChargingRuleDefinition *)calloc(1, sizeof(GxChargingRuleDefinition));
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
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list =  (GxFlowInformation *)calloc(1,sizeof(GxFlowInformation));
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].presence.flow_description =  PRESENT;
        char rule[300]="permit out udp from any to assigned";
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_description.len =strlen(rule);
        strcpy((char *)rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_description.val, rule);

        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].presence.flow_direction = PRESENT;
        rar_request.data.cp_rar.charging_rule_install.list[0].charging_rule_definition.list[0].flow_information.list[0].flow_direction = BIDIRECTIONAL;

        msglen = gx_rar_calc_length(&rar_request.data.cp_rar);
        rar_buffer = (char *)calloc(1, msglen+sizeof(rar_request.msg_type)+4);
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
    for(int i=0;i<256;i++) {
        gtp_out_mock_handler[i] = NULL;
        gtp_in_mock_handler[i] = NULL;
	}

    // enable this hook to feed outgoing messages to custom handler 
    //gtp_out_mock_handler[GTP_CREATE_BEARER_REQ] =  handle_mock_create_bearer_request_msg;
}

void 
init_pfcp_mock_interface(void)
{
    for(int i=0;i<256;i++) {
        pfcp_out_mock_handler[i] = NULL;
        pfcp_in_mock_handler[i] = NULL;
	}

    // enable this hook to feed outgoing messages to custom handler 
}

void 
init_gx_mock_interface(void)
{
    for(int i=0;i<256;i++) {
        gx_out_mock_handler[i] = NULL;
        gx_in_mock_handler[i] = NULL;
	}

    // enable this hook to feed outgoing messages to custom handler 
    //gx_in_mock_handler[GX_RAR_MSG] =  handle_mock_rar_request_msg;
}

void 
handle_unknown_mock_msg(void *event)
{
    LOG_MSG(LOG_ERROR, "Received unknown mock msg event %p",event);
    return;
}

void 
handle_mock_create_bearer_request_msg(void *evt)
{
    int ret;
    msg_info_t *msg = (msg_info_t *)calloc(1, sizeof(msg_info_t));
    outgoing_pkts_event_t *event = (outgoing_pkts_event_t *)evt;

    LOG_MSG(LOG_DEBUG, "Received mock event %p",event);

    ret = decode_create_bearer_req((uint8_t *) event->payload, &msg->rx_msg.cb_req);
    if(ret == 0)
        return;

    uint8_t  cbrsp_pkt[1000];
    uint16_t cbrsp_len;
    {
        gtp_create_bearer_request_bearer_ctxt_ie_t *cbreq_bc = &msg->rx_msg.cb_req.bearer_contexts;
        create_bearer_rsp_t cbrsp = {0}; 
        cbrsp.header.gtpc.teid_flag =  1;
        cbrsp.header.gtpc.version = 2; 
        cbrsp.header.gtpc.message_type = GTP_CREATE_BEARER_RSP; 
        cbrsp.header.gtpc.message_len = 10; 
        cbrsp.header.teid.has_teid.teid = 0; 
        cbrsp.header.teid.has_teid.seq = msg->rx_msg.cb_req.header.teid.has_teid.seq; 

        set_cause_accepted(&cbrsp.cause, IE_INSTANCE_ZERO); 

        set_ebi(&cbrsp.bearer_contexts.eps_bearer_id, IE_INSTANCE_ZERO, 6);
        set_cause_accepted(&cbrsp.bearer_contexts.cause, IE_INSTANCE_ZERO);
        struct in_addr en_addr; 
        en_addr.s_addr = 0x01010101;
        set_ipv4_fteid(&cbrsp.bearer_contexts.s1u_enb_fteid, GTPV2C_IFTYPE_S1U_ENODEB_GTPU, IE_INSTANCE_ZERO, en_addr, 100); 

        struct in_addr sgw_addr; 
        sgw_addr.s_addr = cbreq_bc->s1u_sgw_fteid.ipv4_address;
        uint32_t teid = cbreq_bc->s1u_sgw_fteid.teid_gre_key;
        set_ipv4_fteid(&cbrsp.bearer_contexts.s1u_sgw_fteid, GTPV2C_IFTYPE_S1U_SGW_GTPU, IE_INSTANCE_ZERO, sgw_addr, teid);
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
        cbrsp.header.gtpc.message_len = htons(cbrsp_len);
        gtpv2c_header_t *cbrsp_header = (gtpv2c_header_t *)cbrsp_pkt;
        cbrsp_header->gtpc.message_len = htons(cbrsp_len);
    }


    {
        struct sockaddr_in peer_sockaddr = {0};
        msg_info_t *cbrsp_event = (msg_info_t *)calloc(1, sizeof(msg_info_t));
        cbrsp_event->peer_addr = peer_sockaddr;
        cbrsp_event->source_interface = S11_IFACE;
        cbrsp_event->msg_type = GTP_CREATE_BEARER_RSP;
        cbrsp_event->raw_buf = calloc(1, event->payload_len);
        memcpy(cbrsp_event->raw_buf, cbrsp_pkt, cbrsp_len);
        queue_stack_unwind_event(GTP_MSG_RECEIVED, (void *)cbrsp_event, process_gtp_msg);
    }
    return ;
}

void 
handle_mock_rar_request_msg(void *msg, uint16_t event)
{
	LOG_MSG(LOG_DEBUG, "RAR mock handler called msg %p, event %d ", msg, event);
	return;
}

void init_mock_test(void)
{
    init_gtp_mock_interface();
    init_pfcp_mock_interface();
    init_gx_mock_interface();

    // thread to generate Test events for protocol stack
    pthread_t readerTest_t;
    pthread_attr_t testattr;
    pthread_attr_init(&testattr);
    pthread_attr_setdetachstate(&testattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&readerTest_t, &testattr, &test_event_thread, NULL);
    pthread_attr_destroy(&testattr);
	return;
}
