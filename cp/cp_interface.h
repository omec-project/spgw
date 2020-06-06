/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
