// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _INTERFACE_CP_H_
#define _INTERFACE_CP_H_
#if 0
#include <inttypes.h>
#include <rte_hash.h>
#endif
#include "vepc_cp_dp_api.h"
#include "vepc_udp.h"

/**
 * @brief  : Writes packet at @tx_buf of length @payload_length to pcap file specified
 *           in @pcap_dumper (global)
 * @param  : payload_length, total length
 * @param  : tx_buf, buffer containg packets
 * @return : Returns nothing
 */
void
dump_pcap(uint16_t payload_length, uint8_t *tx_buf);


#endif /* _INTERFACE_H_ */
