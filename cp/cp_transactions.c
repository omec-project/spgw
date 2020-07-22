// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_malloc.h"
#include "rte_lcore.h" 
#include "trans_struct.h"
#include "cp_transactions.h"
#include "clogger.h"
#include "rte_per_lcore.h"
#include "rte_errno.h"
#include <string.h>
#include "errno.h"
#include "rte_common.h"
#include "pfcp_messages.h"
#include "assert.h"
#include "cp_io_poll.h"
#include "cp_interface.h"
#include "sm_hand.h"
#include "cp_config.h"
#include "trans_struct.h"
#include "ue.h"

extern udp_sock_t my_sock;
extern pcap_dumper_t *pcap_dumper;

static transData_t* start_transaction_timer(void *cb_data, 
                        uint8_t *buf, 
                        uint16_t buf_len,
                        timeout_handler_t cb_timeout,
                        gstimercallback cb_retry);

transData_t*
start_pfcp_session_timer(void *ue_context, 
                        uint8_t *buf, 
                        uint16_t buf_len, 
                        timeout_handler_t cb)
{
    return start_transaction_timer(ue_context, buf, buf_len, cb, pfcp_session_transaction_retry_callback);
}

transData_t*
start_pfcp_node_timer(void *upf_context, 
                      uint8_t *buf, 
                      uint16_t buf_len, 
                      timeout_handler_t cb)
{
    return start_transaction_timer(upf_context, buf, buf_len, cb, pfcp_node_transaction_retry_callback);
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
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate transaction entry entry :"
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__, __LINE__);
		return NULL;
	}
	trans_entry->cb_data = (void *) cb_data; 
	trans_entry->buf_len = buf_len;
	memcpy(trans_entry->buf,(uint8_t *)buf, buf_len);
    trans_entry->timeout_function = cb_timeout;

	if (!init_timer(&trans_entry->rt, cp_config->request_timeout, cb_retry, (void *)trans_entry))
	{
		clLog(clSystemLog, eCLSeverityCritical,"%s:%s:%u =>%s - initialization of trans timer failed erro no %d\n",
				__FILE__, __func__, __LINE__, getPrintableTime(), errno);
		return NULL;
	}

    if (starttimer(&trans_entry->rt) < 0) {
        clLog(clSystemLog, eCLSeverityCritical, "%s:%s:%u Periodic Timer failed to start...\n",
                __FILE__, __func__, __LINE__);
    }
	return(trans_entry);
}

void stop_transaction_timer(transData_t *data)
{
    if(data->rt.ti_id != 0) {
        stoptimer(&data->rt.ti_id);
        deinittimer(&data->rt.ti_id);
        data->rt.ti_id = 0;
    } else {
        printf("Bad timer stop event received \n");
    }
 
    return;
}

void
pfcp_node_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t )
{
    RTE_SET_USED(ti);

#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
    transData_t *data =  (transData_t *) data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */

    if (data->itr_cnt >= 3) {
        if(data->rt.ti_id != 0) {
             stoptimer(&data->rt.ti_id);
             deinittimer(&data->rt.ti_id);
        }
        data->timeout_function(data);
        return;
    }

    upf_context_t *upf_ctxt = (upf_context_t *)(data->cb_data);
    pfcp_timer_retry_send(my_sock.sock_fd_pfcp, data, &upf_ctxt->upf_sockaddr);
    data->itr_cnt++;
    return;
}

void
pfcp_session_transaction_retry_callback(gstimerinfo_t *ti, const void *data_t )
{
    RTE_SET_USED(ti);

#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
    transData_t *data =  (transData_t *) data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */

    if (data->itr_cnt >= 3) {
        if(data->rt.ti_id != 0) {
             stoptimer(&data->rt.ti_id);
             deinittimer(&data->rt.ti_id);
        }
        data->timeout_function(data);
        return;
    }
    ue_context_t *ue_context = (ue_context_t *)(data->cb_data);
    upf_context_t *upf_context = ue_context->upf_context;
    pfcp_timer_retry_send(my_sock.sock_fd_pfcp, data, &upf_context->upf_sockaddr);
    data->itr_cnt++;
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

        clLog(clSystemLog, eCLSeverityDebug, "SPGWC - retransmitted PFCP message \n");

        if (bytes_tx != (int) t_tx->buf_len) {
            clLog(clSystemLog, eCLSeverityCritical, "Transmitted Incomplete Timer Retry Message:"
                    "%u of %d tx bytes : %s\n",
                    t_tx->buf_len, bytes_tx, strerror(errno));
        }
    }
}
