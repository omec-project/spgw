// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "rte_malloc.h"
#include "rte_lcore.h" 
#include "trans_struct.h"
#include "pfcp_transactions.h"
#include "clogger.h"
#include "rte_per_lcore.h"
#include "rte_errno.h"
#include <string.h>
#include "errno.h"
#include "rte_common.h"
#include "pfcp_messages.h"
#include "assert.h"
#include "cp_io_poll.h"

extern udp_sock_t my_sock;

transData_t *create_transaction(upf_context_t *upf_ctxt, uint8_t *buf, uint16_t buf_len)
{
	transData_t *trans_entry = NULL;

	trans_entry = (transData_t *)rte_zmalloc_socket(NULL, sizeof(transData_t),
			RTE_CACHE_LINE_SIZE, rte_socket_id());
	if(trans_entry == NULL )
	{
		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate transaction entry entry :"
				"%s (%s:%d)\n",
				rte_strerror(rte_errno),
				__FILE__, __LINE__);
		return NULL;
	}
	trans_entry->upf_ctxt = (void *) upf_ctxt; 
	trans_entry->buf_len = buf_len;
	memcpy(trans_entry->buf,(uint8_t *)buf, buf_len);
    trans_entry->dstIP = upf_ctxt->upf_sockaddr.sin_addr.s_addr;
    trans_entry->msg_type = PFCP_ASSOCIATION_SETUP_REQUEST;
	return(trans_entry);
}

bool
start_transaction_timer(transData_t *trans, uint32_t timeout_ms, gstimercallback cb)
{
	if (!init_timer(&trans->rt, timeout_ms, cb, (void *)trans))
	{
		clLog(clSystemLog, eCLSeverityCritical,"%s:%s:%u =>%s - initialization of trans timer failed erro no %d\n",
				__FILE__, __func__, __LINE__, getPrintableTime(), errno);
		return false;
	}
	return true;
}

transData_t *get_pfcp_transaction(void)
{
    // search and return pfcp transaction 
    return NULL;
}


void delete_pfcp_transaction(void)
{
    // search  transaction 
    // stop timers
    // delete transaction from table 
     
    return;
}

void
transaction_timeout_callback(gstimerinfo_t *ti, const void *data_t )
{
        RTE_SET_USED(ti);

        assert(0);
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
        transData_t *data =  (transData_t *) data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */


        if (data->itr_cnt >= 3) {
                // assert(data->iface == PFCP); 
                switch(data->msg_type)
                {
                    case PFCP_ASSOCIATION_SETUP_REQUEST: 
                        break;
                }
               if(data->rt.ti_id != 0) {
                    stoptimer(&data->rt.ti_id);
                    deinittimer(&data->rt.ti_id);
                    /* free peer data when timer is de int */
                    rte_free(data);
                }
                return;
        }
#if 0
        /* timer retry handler */
        switch(data->portId) {
                case PFCP_IFACE:
                        pfcp_timer_retry_send(my_sock.sock_fd_pfcp, data);
                        break;
                default:
                        break;
        }
#endif
        data->itr_cnt++;
        return;
}

