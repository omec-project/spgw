// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "trans_struct.h"
#include "cp_transactions.h"
#include <string.h>
#include "errno.h"
#include "pfcp_messages.h"
#include "assert.h"
#include "cp_io_poll.h"
#include "cp_interface.h"
#include "sm_hand.h"
#include "spgw_config_struct.h"
#include "trans_struct.h"
#include "ue.h"
#include "proc_struct.h"
#include "spgw_cpp_wrapper.h"
#include "cp_log.h"


extern pcap_dumper_t *pcap_dumper;

static transData_t* start_transaction_timer(void *cb_data, 
                        uint8_t *buf, 
                        uint16_t buf_len,
                        timeout_handler_t cb_timeout,
                        gstimercallback cb_retry);

transData_t*
start_response_wait_timer(void *context, 
                        uint8_t *buf, 
                        uint16_t buf_len, 
                        timeout_handler_t cb)
{
    return start_transaction_timer(context, buf, buf_len, cb, transaction_retry_callback);
}

transData_t*
restart_response_wait_timer(transData_t *trans_entry)
{
	if (!init_timer(&trans_entry->rt, cp_config->request_timeout, transaction_retry_callback, (void *)trans_entry)) {
		LOG_MSG(LOG_ERROR,"initialization of trans timer failed erro no %d", errno);
		return NULL;
	}

    if (starttimer(&trans_entry->rt) < 0) {
        LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
        return NULL;
    }

    return trans_entry;
}

transData_t*
start_pfcp_response_wait_timer(void *context, 
                        uint8_t *buf, 
                        uint16_t buf_len, 
                        timeout_handler_t cb)
{
    return start_transaction_timer(context, buf, buf_len, cb, pfcp_node_transaction_retry_callback);
}

static transData_t* 
start_transaction_timer(void *cb_data, 
                        uint8_t *buf, 
                        uint16_t buf_len,
                        timeout_handler_t cb_timeout,
                        gstimercallback cb_retry)
{
	transData_t *trans_entry = NULL;

	trans_entry = (transData_t *)calloc(1, sizeof(transData_t));
	if(trans_entry == NULL )
	{
		LOG_MSG(LOG_ERROR, "Failure to allocate transaction entry ");
		return NULL;
	}
	trans_entry->cb_data = (void *) cb_data; 
	trans_entry->buf_len = buf_len;
	memcpy(trans_entry->buf,(uint8_t *)buf, buf_len);
    trans_entry->timeout_function = cb_timeout;

	if (!init_timer(&trans_entry->rt, cp_config->request_timeout, cb_retry, (void *)trans_entry)) {
		LOG_MSG(LOG_ERROR,"initialization of trans timer failed erro no %d", errno);
		return NULL;
	}

    if (starttimer(&trans_entry->rt) < 0) {
        LOG_MSG(LOG_ERROR, "Periodic Timer failed to start...");
    }
	return(trans_entry);
}

void stop_transaction_timer(transData_t *data)
{
    if(data->rt.ti_id != 0) {
        stoptimer(&data->rt.ti_id);
        deinittimer(&data->rt.ti_id);
        data->rt.ti_id = 0;
    } 
    return;
}

bool is_transaction_timer_started(transData_t *data)
{
    if(data->rt.ti_id == 0) {
        return false;
    }
    return true;
}

void
pfcp_node_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t )
{

    transData_t *data =  (transData_t *) data_t;

    if (data->itr_cnt >= 3) {
        if(data->rt.ti_id != 0) {
             stoptimer(&data->rt.ti_id);
             deinittimer(&data->rt.ti_id);
             data->rt.ti_id = 0;
        }
        data->timeout_function(data);
        return;
    }

    upf_context_t *upf_ctxt = (upf_context_t *)(data->cb_data);
    pfcp_timer_retry_send(my_sock.sock_fd_pfcp, data, &upf_ctxt->upf_sockaddr);
    data->itr_cnt++;
    LOG_MSG(LOG_NEVER, " timeinfo %p ", ti);
    return;
}

void
transaction_retry_callback(gstimerinfo_t *ti, const void *data_t )
{
    transData_t *data =  (transData_t *) data_t;
    if(IS_TRANS_DELAYED_DELETE(data)) {
        // transaction is marked for free
        return;
    }
    if(data->rt.ti_id != 0) {
        stoptimer(&data->rt.ti_id);
        deinittimer(&data->rt.ti_id);
        data->rt.ti_id = 0;
    }
    data->timeout_function(data->cb_data);
    LOG_MSG(LOG_NEVER, " timeinfo %p ", ti);
    return;
}

/**
 * @brief  : Util to send or dump pfcp messages
 * @param  : fd, interface indentifier
 * @param  : t_tx, buffer to store data for peer node
 * @return : void
 */
void pfcp_timer_retry_send(int fd, transData_t *t_tx, struct sockaddr_in *peer)
{
	int bytes_tx;
	if (pcap_dumper) {
		dump_pcap(t_tx->buf_len, t_tx->buf);
    } else {
        bytes_tx = sendto(fd, t_tx->buf, t_tx->buf_len, 0,
                (struct sockaddr *)peer, sizeof(struct sockaddr_in));

        LOG_MSG(LOG_INFO,"SPGWC - retransmitted PFCP message bytes %d ", bytes_tx);

        if (bytes_tx != (int) t_tx->buf_len) {
            LOG_MSG(LOG_ERROR, "Transmitted Incomplete Timer Retry Message:"
                    "%u of %d tx bytes : %s",
                    t_tx->buf_len, bytes_tx, strerror(errno));
        }
    }
}

/* Unlink transaction from procedure, remote transacton from table */
void 
cleanup_gtpc_trans(transData_t *gtpc_trans)
{
    proc_context_t *proc = (proc_context_t *)gtpc_trans->proc_context;
    uint32_t seq_num = gtpc_trans->sequence; 
    uint32_t trans_addr;
    uint16_t port_num;
    if(IS_TRANS_SELF_INITIATED(gtpc_trans)) {
        port_num = my_sock.s11_sockaddr.sin_port;
        trans_addr = my_sock.s11_sockaddr.sin_addr.s_addr;
    } else {
        /* Only MME initiated transactions as of now */
        port_num = proc->gtpc_trans->peer_sockaddr.sin_port; 
        trans_addr = proc->gtpc_trans->peer_sockaddr.sin_addr.s_addr; 
    }
    transData_t *trans = (transData_t *)delete_gtp_transaction(trans_addr, port_num, seq_num);
    if(trans != NULL) {
        assert(gtpc_trans == trans);
    }
    stop_transaction_timer(gtpc_trans);
    delayed_free(gtpc_trans);

    if(proc != NULL) {
        proc->gtpc_trans = NULL;
    }
}

/* Unlink transaction from procedure, remote transacton from table */
void 
cleanup_pfcp_trans(transData_t *pfcp_trans)
{
    proc_context_t *proc = (proc_context_t *)pfcp_trans->proc_context;
    uint32_t trans_addr;
    uint16_t port_num;
    uint32_t seq_num = pfcp_trans->sequence; 
    if(IS_TRANS_SELF_INITIATED(pfcp_trans)) {
        trans_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
        port_num = my_sock.pfcp_sockaddr.sin_port;
    } else {
        trans_addr = pfcp_trans->peer_sockaddr.sin_addr.s_addr;
        port_num = pfcp_trans->peer_sockaddr.sin_port;
    }
    transData_t *trans = (transData_t *)delete_pfcp_transaction(trans_addr, port_num, seq_num);
    if(trans != NULL) {
        assert(pfcp_trans == trans);
    }
    stop_transaction_timer(pfcp_trans);
    delayed_free(pfcp_trans);

    if(proc != NULL) {
        proc->pfcp_trans = NULL;
    }
}

/* Unlink transaction from procedure, remote transacton from table */
void 
cleanup_gx_trans(transData_t *gx_trans)
{
    LOG_MSG(LOG_DEBUG,"Cleanup transaction gx ");
    proc_context_t *proc = (proc_context_t *)gx_trans->proc_context;
    uint32_t seq_num = gx_trans->sequence; 
    transData_t *trans = (transData_t *)delete_gx_transaction(seq_num);
    if(trans != NULL) {
        assert(gx_trans == trans);
    }
    stop_transaction_timer(gx_trans);
    delayed_free(gx_trans);

    if(proc != NULL) {
        proc->gx_trans = NULL;
    }
}

void delayed_free(transData_t *trans)
{

    if(IS_TRANS_DELAYED_DELETE(trans)) {
        LOG_MSG(LOG_DEBUG,"Transaction already marked for delayed delete %p ",trans);
        return;
    }
    SET_TRANS_DELAYED_DELETE(trans);
    add_delayed_free_memory_task(trans);
}
