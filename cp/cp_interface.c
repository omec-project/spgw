// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <rte_common.h>
#include <rte_eal.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_jhash.h>
#include <rte_cfgfile.h>
#include <rte_debug.h>

#include "util.h"
#include "cp_interface.h"
#include "cp_io_poll.h"
#include "clogger.h"

//#include "acl_dp.h"
//#include "meter.h"
//#include "gtpv2c_ie.h"

#include "gtpv2c.h"
#include "ipc_api.h"

#ifdef USE_AF_PACKET
#include <libmnl/libmnl.h>
#endif
#ifdef SGX_CDR
	#define DEALERIN_IP "dealer_in_ip"
	#define DEALERIN_PORT "dealer_in_port"
	#define DEALERIN_MRENCLAVE "dealer_in_mrenclave"
	#define DEALERIN_MRSIGNER "dealer_in_mrsigner"
	#define DEALERIN_ISVSVN "dealer_in_isvsvn"
	#define DP_CERT_PATH "dp_cert_path"
	#define DP_PKEY_PATH "dp_pkey_path"
#endif /* SGX_CDR */

#ifdef TIMER_STATS
#ifdef AUTO_ANALYSIS
extern void print_perf_statistics(void);
#endif /* AUTO_ANALYSIS */
#endif /* TIMER_STATS */

extern struct ipc_node *basenode;
extern struct rte_hash *heartbeat_recovery_hash;

struct rte_hash *node_id_hash;

#define IFACE_FILE "../config/interface.cfg"
#define SET_CONFIG_IP(ip, file, section, entry) \
do {\
	entry = rte_cfgfile_get_entry(file, section, #ip);\
	if (entry == NULL)\
	rte_panic("%s not found in %s", #ip, IFACE_FILE);\
	if (inet_aton(entry, &ip) == 0)\
	rte_panic("Invalid %s in %s", #ip, IFACE_FILE);\
} while (0)
#define SET_CONFIG_PORT(port, file, section, entry) \
do {\
	entry = rte_cfgfile_get_entry(file, section, #port);\
	if (entry == NULL)\
	rte_panic("%s not found in %s", #port, IFACE_FILE);\
	if (sscanf(entry, "%"SCNu16, &port) != 1)\
	rte_panic("Invalid %s in %s", #port, IFACE_FILE);\
} while (0)


