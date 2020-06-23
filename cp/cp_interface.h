// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef _INTERFACE_CP_H_
#define _INTERFACE_CP_H_
#include <pcap.h>
#include "vepc_cp_dp_api.h"
#include "trans_struct.h"
#include "cp_peer_struct.h"

extern pcap_dumper_t *pcap_dumper;
/**
 * @brief  : Writes packet at @tx_buf of length @payload_length to pcap file specified
 *           in @pcap_dumper (global)
 * @param  : payload_length, total length
 * @param  : tx_buf, buffer containg packets
 * @return : Returns nothing
 */
void
dump_pcap(uint16_t payload_length, uint8_t *tx_buf);


void
gtpc_timer_retry_send(int fd, peerData_t *t_tx);

void
pfcp_timer_retry_send(int fd, peerData_t *t_tx);

void
pfcp_timer_retry_send_new(int fd, transData_t *t_tx);

#endif /* _INTERFACE_H_ */
