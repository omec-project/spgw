// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#include <unistd.h>
#include <signal.h>

#include "ipc_api.h"
#include "gx.h"
#include "cp_log.h"

extern int g_gx_client_sock;
uint8_t logging_level=LOG_DEBUG;
gx_trans_data_t *gx_trans_list = NULL;

int done = 0;

void signal_handler(int sig)
{
	done = 1;
}

int fdinit(const char *fdcfg)
{
	/* Initialize the core freeDiameter library */
	CHECK_FCT_DO( fd_core_initialize(), return FD_REASON_CORE_INIT_FAIL );
	/* Parse the configuration file */
	CHECK_FCT_DO( fd_core_parseconf(fdcfg), return FD_REASON_PARSECONF_FAIL );
	return FD_REASON_OK;
}

int fdstart()
{
	/* Start freeDiameter */
	CHECK_FCT_DO( fd_core_start(), return FD_REASON_PARSECONF_FAIL );
	return FD_REASON_OK;
}

/**
 * @brief  : Parse fd configuration
 * @param  : filename , config file name
 * @param  : peer_name , peer node name
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
parse_fd_config(const char *filename, char *peer_name)
{
	FILE *gx_fd = NULL;
	char data[1024] = {0};
	char *token = NULL;
	char *token1 = NULL;
	size_t str_len = 0;

	if((gx_fd = fopen(filename, "r")) <= 0) {
		LOG_MSG(LOG_ERROR, "ERROR :unable to read [ %s ] file",filename);
		return -1;
	}
	fseek(gx_fd, 0L, SEEK_SET);

	while((fgets(data, 256, gx_fd)) != NULL) {
		if(data[0]  == '#') {
			continue;
		}
		if(strstr(data, CONNECTPEER) != NULL) {
				token = strchr(data, '"');
				if(token != NULL){
					token1 = strchr(token+1, '"');
					str_len = token1 - token;
					memcpy(peer_name, token+1, str_len-1);
				}
				fclose(gx_fd);
				return 0;
		}

	}
	fclose(gx_fd);
	return -1;
}

int main(int argc, char **argv)
{
	int rval = 0;
	const char *fdcfg = "config/gx.conf";
	char peer_name[256] = {0};

    gx_trans_list = (gx_trans_data_t *)calloc(1, sizeof(gx_trans_data_t));
    gx_trans_list->rar_seq_num = 1;

	LOG_MSG(LOG_INIT, "Registering signal handler...");
	if ( signal(SIGINT, signal_handler) == SIG_ERR )
	{
		LOG_MSG(LOG_ERROR,"Cannot catch SIGINT");
		return 1;
	}

	LOG_MSG(LOG_INIT, "Initializing freeDiameter...");
	if ( (rval = fdinit(fdcfg)) != FD_REASON_OK )
	{
		LOG_MSG(LOG_ERROR,"Failure (%d) in fdinit()", rval);
		return 1;
	}

	LOG_MSG(LOG_INIT, "Calling gxInit()...");
	if ( (rval = gxInit()) != FD_REASON_OK )
	{
		LOG_MSG(LOG_ERROR, "Failure (%d) in gxInit()", rval);
		return 1;
	}

	LOG_MSG(LOG_INIT,"Calling gxRegistger()...");
	if ( (rval = gxRegister()) != FD_REASON_OK )
	{
		LOG_MSG(LOG_ERROR,"Failure (%d) in gxRegister()", rval);
		return 1;
	}

	LOG_MSG(LOG_INIT, "Starting freeDiameter...");
	if ( (rval = fdstart()) != FD_REASON_OK )
	{
		LOG_MSG(LOG_ERROR,"Failure (%d) in fdstart()", rval);
		return 1;
	}

	if(parse_fd_config(fdcfg, peer_name) < 0 ) {
		LOG_MSG(LOG_ERROR, "unable to read [ %s ] file ",fdcfg);
		return -1;
	}

	LOG_MSG(LOG_INIT, "Waiting to connect to [%s] ", peer_name);
	while(1){
		struct peer_hdr *peer;
		sleep(1);
		if ( ! fd_peer_getbyid(peer_name, strlen(peer_name), 1, &peer ) ){
			int state = fd_peer_get_state(peer);
			if ( state == STATE_OPEN || state == STATE_OPEN_NEW ) {
				break;
			}
		}
		if(done == 1) {
			close_ipc_channel(g_gx_client_sock);
			fd_core_shutdown();
			fd_core_wait_shutdown_complete();
			return -1;
		}
	}

	LOG_MSG(LOG_INIT, "Opening unix socket...");
	if ( (rval = unixsock()) != FD_REASON_OK )
	{
		LOG_MSG(LOG_ERROR,"Failure (%d) in unixsock()", rval);
		return 1;
	}


	while (!done)
		sleep(1);

	close_ipc_channel(g_gx_client_sock);
	fd_core_shutdown();
	fd_core_wait_shutdown_complete();

	return 0;
}
