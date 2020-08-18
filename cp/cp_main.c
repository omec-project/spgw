// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <stdio.h>
#include <getopt.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_cfgfile.h>

#include "gw_adapter.h"
#include "clogger.h"
#include "cp_init.h"
#include "cp_main.h"
#include "cp_stats.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "cp_io_poll.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp.h"
#include <sys/stat.h>
#include "cp_config.h"
#include "cp_config_apis.h"
#include "cp_config_defs.h"
#include "ipc_api.h"
#include "spgw_cpp_wrapper.h"
#include "cp_timer.h"
#include "cp_peer.h"
#include "tables/tables.h"
#include "cp_io_poll.h"

#ifdef USE_DNS_QUERY
#include "cdnshelper.h"
#endif /* USE_DNS_QUERY */

#ifdef USE_CSID
#include "csid_struct.h"
#endif /* USE_CSID */

#define LOG_LEVEL_SET      (0x0001)
#define IP_POOL_IP_SET     (0x0080)
#define IP_POOL_MASK_SET   (0x0100)
#define APN_NAME_SET	   (0x0200)

pcap_t *pcap_reader;
pcap_dumper_t *pcap_dumper;
uint32_t start_time;
char *config_update_base_folder = NULL;
bool native_config_folder = false;

/* We should move all the config inside this structure eventually
 * config is scattered all across the place as of now
 */
uint8_t rstCnt = 0;

#ifdef USE_CSID
uint16_t local_csid = 0;
#endif /* USE_CSID */

struct cp_params cp_params;
clock_t cp_stats_execution_time;
_timer_t st_time;

/**
 *
 * @brief  : Parses non-dpdk command line program arguments for control plane
 * @param  : argc, number of arguments
 * @param  : argv, array of c-string arguments
 * @return : Returns nothing
 */
static void
parse_arg(int argc, char **argv)
{
    char errbuff[PCAP_ERRBUF_SIZE];
    int c = 0;
    pcap_t *pcap;

    const struct option long_options[] = {
        {"pcap_file_in", required_argument, NULL, 'x'},
        {"pcap_file_out", required_argument, NULL, 'y'},
        {"log_level",   required_argument, NULL, 'l'},
        {"config_update_base_folder",optional_argument, NULL, 'f'},
        {0, 0, 0, 0}
    };

    do {
        int option_index = 0;

        c = getopt_long(argc, argv, "x:y:l:f:", long_options,
                &option_index);

        if (c == -1)
            break;

        switch (c) {

            case 'x':
                {
                    pcap_reader = pcap_open_offline(optarg, errbuff);
                    break;
                }

            case 'y':
                {
                    pcap = pcap_open_dead(DLT_EN10MB, UINT16_MAX);
                    pcap_dumper = pcap_dump_open(pcap, optarg);
                    s11_pcap_fd = pcap_fileno(pcap);
                    break;
                }

            case 'f':
                {
                    config_update_base_folder = calloc(1, 128);
                    if (config_update_base_folder == NULL)
                        rte_panic("Unable to allocate memory for config_update_base_folder var!\n");
                    strcpy(config_update_base_folder, optarg);
                    break;
                }

            default:
                // No crash for unknown args 
                printf("\nUnknown argument %c - %s.", c, argv[optind]);
                break;
        }
    } while (c != -1);

    /* Lets put default values if some configuration is missing */
    if (config_update_base_folder == NULL) {
        config_update_base_folder = (char *) calloc(1, 128);
        if (config_update_base_folder == NULL)
            rte_panic("Unable to allocate memory for config_update_base_folder!\n");
        strcpy(config_update_base_folder, CP_CONFIG_FOLDER);
        native_config_folder = true;
    }
}

/**
 * @brief  : initializes the core assignments for various control plane threads
 * @param  : No param
 * @return : Returns nothing
 */
static void
init_cp_params(void) 
{
    unsigned last_lcore = rte_get_master_lcore();

    cp_params.stats_core_id = rte_get_next_lcore(last_lcore, 1, 0);
    if (cp_params.stats_core_id == RTE_MAX_LCORE)
        clLog(clSystemLog, eCLSeverityCritical, "Insufficient cores in coremask to "
                "spawn stats thread\n");
    last_lcore = cp_params.stats_core_id;

}


/**
 * @brief  : Main function - initializes dpdk environment, parses command line arguments,
 *           calls initialization function, and spawns stats and control plane function
 * @param  : argc, number of arguments
 * @param  : argv, array of c-string arguments
 * @return : returns 0
 */
int
main(int argc, char **argv)
{
    int ret;

    start_time = current_ntp_timestamp();

    /* VS: Increment the restart counter value after starting control plane */
    rstCnt = update_rstCnt();

    TIMER_GET_CURRENT_TP(st_time);
    recovery_time_into_file(start_time);

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_panic("Cannot init EAL\n");

    int state = mkdir(DEFAULT_STATS_PATH, S_IRWXU);
    if (state && errno != EEXIST) {
        rte_exit(EXIT_FAILURE, "Failed to create directory %s: %s\n",
                DEFAULT_STATS_PATH, strerror(errno));
    }

    parse_arg(argc - ret, argv + ret);

    // this parses file and allocates cp_config  
    init_config();

    init_cp();

    init_cp_params();

    init_cpp_tables(); 

    create_heartbeat_hash_table();

#ifdef DELETE
    create_associated_upf_hash();
#endif

    /* Make a connection between control-plane and gx_app */
    if(cp_config->gx_enabled == true)
    {
        if(cp_config->cp_type != SGWC)
        {
            start_cp_app();
        }
    }

#ifdef SYNC_STATS
    stats_init();
    init_stats_hash();
#endif /* SYNC_STATS */

    if (cp_params.stats_core_id != RTE_MAX_LCORE)
        rte_eal_remote_launch(do_stats, NULL, cp_params.stats_core_id);

	echo_table_init();

    /* Create thread for handling for sending echo req to its peer node */
    rest_thread_init();


    init_pfcp_tables();

#ifdef USE_CSID
    init_fqcsid_hash_tables();
#endif /* USE_CSID */
    while (1) {
        iface_process_ipc_msgs();
    }

    /* TODO: Move this call in appropriate place */
    /* clear_heartbeat_hash_table(); */
    return 0;
}

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        if (my_sock.gx_app_sock > 0)
            close_ipc_channel(my_sock.gx_app_sock);
#ifdef SYNC_STATS
        retrive_stats_entry();
        close_stats();
#endif /* SYNC_STATS */

        gst_deinit();

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


