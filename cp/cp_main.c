// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0


#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include "cp_init.h"
#include "cp_main.h"
#include "sm_struct.h"
#include "sm_structs_api.h"
#include "cp_io_poll.h"
#include "pfcp_cp_util.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp.h"
#include <sys/stat.h>
#include "spgw_config_struct.h"
#include "cp_config_apis.h"
#include "cp_config_defs.h"
#include "ipc_api.h"
#include "spgw_cpp_wrapper.h"
#include "cp_timer.h"
#include "cp_peer.h"
#include "cp_io_poll.h"
//#include "cdnshelper.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_interface.h"
#include "gx_interface.h"
#include "cp_test.h"
#include "cp_log.h"
#include "assert.h"
#include "rte_eal.h"

#ifdef USE_CSID
#include "csid_struct.h"
#endif /* USE_CSID */

#define LOG_LEVEL_SET      (0x0001)
#define IP_POOL_IP_SET     (0x0080)
#define IP_POOL_MASK_SET   (0x0100)
#define APN_NAME_SET	   (0x0200)

uint8_t logging_level;
pcap_t *pcap_reader = NULL;
pcap_dumper_t *pcap_dumper;
char *config_update_base_folder = NULL;
bool native_config_folder = false;

#ifdef USE_CSID
uint16_t local_csid = 0;
#endif /* USE_CSID */

struct cp_params cp_params;
// _timer_t st_time; DELETE_CODE

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
        {"config_update_base_folder",required_argument, NULL, 'f'},
        {0, 0, 0, 0}
    };

    do {
        int option_index = 0;

        c = getopt_long(argc, argv, "x:y:l:f:", long_options,
                &option_index);

        if (c == -1) {
            LOG_MSG(LOG_ERROR, "done with  parsing configuration");
            break;
        }

        switch (c) {
            case 'x':
                {
                    LOG_MSG(LOG_ERROR,"argument -x pcap reader %s ", argv[optind]);
                    pcap_reader = pcap_open_offline(optarg, errbuff);
                    break;
                }

            case 'y':
                {
                    LOG_MSG(LOG_ERROR,"argument -y %s ", argv[optind]);
                    pcap = pcap_open_dead(DLT_EN10MB, UINT16_MAX);
                    pcap_dumper = pcap_dump_open(pcap, optarg);
                    s11_pcap_fd = pcap_fileno(pcap);
                    break;
                }

            case 'f':
                {
                    LOG_MSG(LOG_ERROR,"argument %s ", optarg);
                    config_update_base_folder = calloc(1, 128);
                    assert(config_update_base_folder != NULL);
                    strcpy(config_update_base_folder, optarg);
                    LOG_MSG(LOG_INIT,"config base folder set to %s ", config_update_base_folder);
                    break;
                }

            default:
                // No crash for unknown args 
                LOG_MSG(LOG_ERROR,"Unknown argument %c - %s.", c, argv[optind]);
                break;
        }
    } while (c != -1);

    /* Lets put default values if some configuration is missing */
    if (config_update_base_folder == NULL) {
        LOG_MSG(LOG_INIT,"using default config base folder");
        config_update_base_folder = (char *) calloc(1, 128);
        assert(config_update_base_folder != NULL);
        strcpy(config_update_base_folder, CP_CONFIG_FOLDER);
        native_config_folder = true;
    }
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
    //int ret;

	set_logging_level("LOG_ERROR");
    LOG_MSG(LOG_INIT, "Starting main thread ");

    //ret = rte_eal_init(argc, argv);
    //assert(ret >= 0);
    // skipping eal argument 
    int i=0;
    for(i=1; i<=argc; i++) {
        if(strcmp(argv[i],"--")==0) {
            break;
        }
    }

    char name[] = "ngic_controlplane"; 
    argv[i] = name;
    LOG_MSG(LOG_INIT, "Starting main thread argv[0] %s ",argv[i]);
    parse_arg(argc-i, argv+i);

    int state = mkdir(DEFAULT_STATS_PATH, S_IRWXU);
    if (state && errno != EEXIST) {
        assert(0);
    }


    init_cp();

#ifdef DELETE
    create_associated_upf_hash();
#endif

#ifdef USE_CSID
    init_fqcsid_hash_tables();
#endif /* USE_CSID */

	if (signal(SIGINT, sig_handler) == SIG_ERR)
        assert(0);

	if (signal(SIGSEGV, sig_handler) == SIG_ERR)
        assert(0);

    while(1) {
		incoming_event_handler(NULL);
    }

    return 0;
}

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        if (my_sock.gx_app_sock > 0)
            close_ipc_channel(my_sock.gx_app_sock);

        gst_deinit();

        assert(0);
    }
    else if (signo == SIGSEGV)
        assert(0);
}


