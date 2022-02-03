// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IPC_API_H
#define IPC_API_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MULTIPLIER 50
#define BUFFSIZE MULTIPLIER * 1024
/**
 * @brief  : Performs Gx Interface Unix socket creation
 * @param  : No param
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
create_ipc_channel(void );

/**
 * @brief  : Performs Gx Socket Bind
 * @param  : sock, GX socket id
 * @param  : sock_addr, socket address info
 * @param  : path, Filepath
 * @return : Returns nothing
 */
void
bind_ipc_channel(int sock, struct sockaddr_un sock_addr,const  char *path);

/**
 * @brief  : Performs Gx_app client connection
 * @param  : sock, GX socket id
 * @param  : sock_addr, socket address info
 * @param  : path, Filepath
 * @return : Returns nothing
 */
void
connect_to_ipc_channel(int sock, struct sockaddr_un sock_addr, const char *path);

/**
 * @brief  : Performs Socket connection accept function
 * @param  : sock, socket id
 * @param  : sock_addr, socket address info
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
accept_from_ipc_channel(int sock, struct sockaddr_un sock_addr);

/**
 * @brief  : Enables Unix Server waiting for Gx_app client connection
 * @param  : sock, socket id
 * @return : Returns nothing
 */
void
listen_ipc_channel(int sock);

/**
 * @brief  : Retrive peer node name
 * @param  : sock, socket id
 * @param  : sock_addr, socket address info
 * @return : Returns nothing
 */
void
get_peer_name(int sock, struct sockaddr_un sock_addr);

/**
 * @brief  : Accept data from created ipc channel
 * @param  : sock, socket id
 * @param  : buf, buffer to store incoming data
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
recv_from_ipc_channel(int sock, char *buf);

/**
 * @brief  : Send data to created ipc channel
 * @param  : sock, socket id
 * @param  : buf, buffer to store data to be sent
 * @param  : len, total data length
 * @return : Returns nothing
 */
void
send_to_ipc_channel(int sock, char *buf, int len);

/**
 * @brief  : Close ipc channel
 * @param  : sock, socket id
 * @return : Returns nothing
 */
void
close_ipc_channel(int sock);

#ifdef __cplusplus
}
#endif
#endif /* IPC_API_H*/
