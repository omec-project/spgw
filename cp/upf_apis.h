// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __UPF_APIS__
#define __UPF_APIS__

#include "upf_struct.h"
#include "spgw_config_struct.h"
#include "cp_timer.h"
#include "cp_proc.h"
#include "sm_struct.h"

int 
create_upf_context(uint32_t upf_ip, upf_context_t **upf_ctxt); 

void
upf_down_event(uint32_t upf_ip);

void 
initiate_all_pfcp_association(void);

void 
schedule_pfcp_association(uint16_t timeout, upf_context_t *upf);

void
start_upf_procedure(proc_context_t *proc, msg_info_t *msg);

void 
end_upf_procedure(proc_context_t *proc_ctxt);

struct in_addr 
native_linux_name_resolve(const char *name);

upf_context_t*
get_upf_context(user_plane_profile_t *upf_profile); 

void
handleUpfAssociationTimeoutEvent(void *data, uint16_t event);

void 
upfAssociationTimerCallback( gstimerinfo_t *ti, const void *data_t );


#endif
