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
#include "up_interface.h"
#include "up_io_poll.h"
#include "clogger.h"

//#include "acl_dp.h"
//#include "meter.h"
//#include "gtpv2c_ie.h"

#include "up_acl.h"

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

/*
 * UDP Setup
 */
udp_sock_t my_sock;

/* VS: ROUTE DISCOVERY */
extern int route_sock;


struct in_addr dp_comm_ip;
struct in_addr cp_comm_ip;
uint16_t dp_comm_port;
uint16_t cp_comm_port;


#ifdef TIMER_STATS
#ifdef AUTO_ANALYSIS
extern void print_perf_statistics(void);
#endif /* AUTO_ANALYSIS */
#endif /* TIMER_STATS */

extern struct ipc_node *basenode;
extern struct rte_hash *heartbeat_recovery_hash;

struct rte_hash *node_id_hash;

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))






/**
 * @brief  : Init listen socket.
 * @param  : No param
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
udp_init_dp_socket(void)
{
	if (__create_udp_socket(cp_comm_ip, cp_comm_port, dp_comm_port,
				&my_sock) < 0)
		rte_exit(EXIT_FAILURE, "Create DP UDP Socket "
				"Failed for IP %s:%d!!!\n",
				inet_ntoa(cp_comm_ip), cp_comm_port);
	return 0;
}


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

/**
 * @brief  : Read interface configuration form config file
 * @param  : No param
 * @return : Returns nothing
 */
static
void read_interface_config(void)
{
		struct rte_cfgfile *file = rte_cfgfile_load(IFACE_FILE, 0);
		const char *file_entry;

		if (file == NULL)
			rte_exit(EXIT_FAILURE, "Cannot load configuration profile %s\n",
					IFACE_FILE);

		SET_CONFIG_IP(dp_comm_ip, file, "0", file_entry);
		SET_CONFIG_PORT(dp_comm_port, file, "0", file_entry);

		SET_CONFIG_IP(cp_comm_ip, file, "0", file_entry);
		SET_CONFIG_PORT(cp_comm_port, file, "0", file_entry);

#ifdef SGX_CDR
		app.dealer_in_ip = rte_cfgfile_get_entry(file, "0",
				DEALERIN_IP);
		app.dealer_in_port = rte_cfgfile_get_entry(file, "0",
				DEALERIN_PORT);
		app.dealer_in_mrenclave = rte_cfgfile_get_entry(file, "0",
				DEALERIN_MRENCLAVE);
		app.dealer_in_mrsigner = rte_cfgfile_get_entry(file, "0",
				DEALERIN_MRSIGNER);
		app.dealer_in_isvsvn = rte_cfgfile_get_entry(file, "0",
				DEALERIN_ISVSVN);
		app.dp_cert_path = rte_cfgfile_get_entry(file, "0",
				DP_CERT_PATH);
		app.dp_pkey_path = rte_cfgfile_get_entry(file, "0",
				DP_PKEY_PATH);
#endif /* SGX_CDR */
}


/**
 * @brief Initialize iface message passing
 *
 * This function is not thread safe and should only be called once by DP.
 */
void iface_module_constructor(void)
{
		/* Read and store ip and port for socket communication between cp and
		 * dp*/
		read_interface_config();

		/* User Plane is starting UDP socket to received PFCP packets */
		printf("Opening up socket at dp\n");
		clLog(clSystemLog, eCLSeverityMajor, "IFACE: DP Initialization\n");
		create_udp_socket(dp_comm_ip, dp_comm_port, &my_sock);

}

void sig_handler(int signo)
{
		if (signo == SIGINT) {
#ifdef USE_REST
			gst_deinit();
#endif /* USE_REST */

			close(route_sock);
#ifdef USE_AF_PACKET
      		mnl_socket_close(mnl_sock);
#endif /* USE_AF_PACKET */
#ifdef TIMER_STATS
#ifdef AUTO_ANALYSIS
			print_perf_statistics();
#endif /* AUTO_ANALYSIS */
#endif /* TIMER_STATS */
			rte_exit(EXIT_SUCCESS, "received SIGINT\n");
		}
	else if (signo == SIGSEGV)
		rte_panic("received SIGSEGV\n");
}

