// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <signal.h>
#include <errno.h>
#include "cp_init.h"
#include "pfcp_cp_set_ie.h"
#include "pfcp_cp_session.h"
#include "pfcp_cp_association.h"
#include "cp_timer.h"
#include "sm_struct.h"
#include "cp_config_apis.h"
#include "upf_apis.h"
#include "spgw_config_struct.h"
#include "cp_interface.h"
#include "cp_transactions.h"
#include "gtpv2_internal.h"
#include "cp_io_poll.h"
#include "gtpv2_interface.h"
#include "pfcp_cp_interface.h"
#include "cp_log.h"
#include "cp_test.h"
#include "cp_timer.h"
#include "pfcp_cp_util.h"
#include "assert.h"
#include "gx_interface.h"

/* We should move all the config inside this structure eventually
 * config is scattered all across the place as of now
 */
uint8_t rstCnt = 0;
uint32_t start_time;
int s11_pcap_fd = -1;
extern pcap_t *pcap_reader;
extern pcap_dumper_t *pcap_dumper;

uint8_t s11_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s5s8_rx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t s5s8_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t pfcp_tx_buf[MAX_GTPV2C_UDP_LEN];
uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

udp_sock_t my_sock;

static void 
init_thread_to_thread_socket(void)
{
	int ret;
    struct sockaddr_in local_sockaddr;
    const char *loopback = "127.0.0.1"; 
    struct in_addr local_ip;		/* Internet address.  */
    inet_aton(loopback, &local_ip);

	int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
	my_sock.sock_fd_local = local_fd;

	if (local_fd < 0)
        assert(0);

	bzero(local_sockaddr.sin_zero, sizeof(local_sockaddr.sin_zero));
	local_sockaddr.sin_family = AF_INET;
	local_sockaddr.sin_port = htons(9090); // any random port is good enough  
	local_sockaddr.sin_addr = local_ip;

    int flag = 1;
    if (-1 == setsockopt(local_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        LOG_MSG(LOG_ERROR,"setsockopt fail");
    }

	ret = bind(local_fd, (struct sockaddr *) &local_sockaddr, sizeof(struct sockaddr_in));

	LOG_MSG(LOG_INIT,"init local local thread to thread socket opened ");

	if (ret < 0) {
        assert(0);
	}
    my_sock.loopback_sockaddr = local_sockaddr; 
}

void* delayed_task_handler(void *data)
{
    LOG_MSG(LOG_DEBUG,"Started delayed_task_handler %p", data);
    /* Free transaction here */
    while(1) {
        int size=0;
        int time = 1000000; 
        void *temp = delete_delayed_free_memory_task(&size);
        if(temp != NULL) {
            free(temp);
            if(size > 100) {
                size = 100;
                time = 10;
            }
        }
        for(int i=0;i<size; i++) {
            int size1;
            void *temp1 = delete_delayed_free_memory_task(&size1);
            if(temp1 != NULL) {
                free(temp1);
            }
            usleep(1); 
        }
        usleep(time);
    }
    return NULL;
}

void config_callback_func(config_callback_t *val) {
    LOG_MSG(LOG_DEBUG, "config_callback_func");
    if (val != NULL) {
        switch (val->action) {
            case DISABLE_UPF:
                LOG_MSG(LOG_DEBUG, "DISABLE_UPF : upf_addr=%s",
                        val->upf_service_name);
                config_disable_upf(val->upf_service_name);
                break;
            default:
                LOG_MSG(LOG_INIT,
                         "Config action not supported=%u",
                          val->action);
        }
        free(val);
    }
}

/**
 * @brief  : Initializes Control Plane data structures, packet filters, and calls for the
 *           Data Plane to create required tables
 */
void init_cp(void)
{
    start_time = current_ntp_timestamp();

    rstCnt = update_rstCnt();

    init_timer_thread();

    init_cpp_tables(); 

    // this parses file and allocates cp_config  
    init_config();

    setup_prometheus(cp_config->prom_port);

    setup_webserver(cp_config->webserver_port,
                    &config_callback_func);

	init_pfcp();
	
	init_gtp();

	init_gx();

	init_mock_test();

    init_thread_to_thread_socket();

    pthread_t delay_t;
    pthread_attr_t delayattr;
    pthread_attr_init(&delayattr);
    pthread_attr_setdetachstate(&delayattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&delay_t, &delayattr, &delayed_task_handler, NULL);
    pthread_attr_destroy(&delayattr);

	return;
}

uint8_t update_rstCnt(void)
{
	FILE *fp;
	int tmp;

	if ((fp = fopen(RESTART_CNT_FILE,"rw+")) == NULL){
		if ((fp = fopen(RESTART_CNT_FILE,"w")) == NULL)
			LOG_MSG(LOG_ERROR,"Error! creating cp_rstCnt.txt file");
	}

	if (fscanf(fp,"%u", &tmp) < 0) {
		/* Cur pos shift to initial pos */
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%u\n", ++rstCnt);
		fclose(fp);
		LOG_MSG(LOG_INIT, "Setting restart counter Value of rstcnt=%u", rstCnt);
		return rstCnt;

	}
	/* Cur pos shift to initial pos */
	fseek(fp, 0, SEEK_SET);

	rstCnt = tmp;
	fprintf(fp, "%d\n", ++rstCnt);

	LOG_MSG(LOG_INIT, "Updated restart counter Value of rstcnt=%u", rstCnt);
	fclose(fp);

	return rstCnt;
}

void recovery_time_into_file(uint32_t recov_time)
{
    FILE *fp = NULL;

    if ((fp = fopen(HEARTBEAT_TIMESTAMP, "w+")) == NULL) {
        LOG_MSG(LOG_ERROR, "Unable to open heartbeat recovery file..");
    } else {
        fseek(fp, 0, SEEK_SET);
        fprintf(fp, "%u\n", recov_time);
        fclose(fp);
    }
}

void init_timer_thread(void)
{
	sigset_t sigset;

	/* mask SIGALRM in all threads by default */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGRTMIN);
	sigaddset(&sigset, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	if (!gst_init()) {
		LOG_MSG(LOG_ERROR, "gstimer_init() failed!!");
	}
	return;
}

