// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: Apache-2.0

#include "ipc_api.h"
#include "cp_log.h"

int
create_ipc_channel( void )
{
	/* STREAM - BiDirectional
	  DATAGRAM - uniDirectional */
	int sock ;

	/* Unix Socket Creation and Verification */
	sock = socket( AF_UNIX, SOCK_STREAM, 0);
	if ( sock == -1 ){
		LOG_MSG(LOG_ERROR,"Gx Interface Unix socket creation failed error:%s", strerror(errno));
		/* Greacefull Exit */
		exit(0);
	}
	return sock;
}

void
connect_to_ipc_channel(int sock, struct sockaddr_un sock_addr, const char *path)
{
	int rc = 0;

	socklen_t  len = sizeof(struct sockaddr_un);
	sock_addr.sun_family = AF_UNIX;

	chmod( path, 755 );

	strncpy(sock_addr.sun_path, path, strlen(path));

    do {
        rc = connect( sock, (struct sockaddr *) &sock_addr, len);
        if ( rc == -1){
            LOG_MSG(LOG_ERROR, "Socket connect failed error: %s", strerror(errno));
            //close_ipc_channel( sock );
            /* Greacefull Exit */
            //exit(0);
            sleep(2);
            continue;
        }
        LOG_MSG(LOG_ERROR, "GxApp: Gx_app client connection succesfully connected...!!!");
        return;
    } while(1);
}

void
bind_ipc_channel(int sock, struct sockaddr_un sock_addr,const char *path)
{
	int rc = 0;
	/* Assign specific permission to path file read, write and executable */
	chmod( path, 755 );

	/* Assign Socket family and PATH */
	sock_addr.sun_family = AF_UNIX;
	strncpy(sock_addr.sun_path, path, strlen(path));

	/* Remove the symbolic link of path names */
	unlink(path);
	/* Bind the new created socket to given PATH and verification */
	rc =  bind( sock, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
	if( rc != 0 ){
		close_ipc_channel(sock);
		LOG_MSG(LOG_ERROR,"Gx Socket Bind failed error: %s", strerror(errno));
		/* Greacefull Exit */
		exit(0);
	}
}

int
accept_from_ipc_channel(int sock, struct sockaddr_un sock_addr)
{
	int new_sock = 0;
	socklen_t len ;
	len = sizeof(sock_addr);

    LOG_MSG(LOG_INIT, "accept_from_ipc_channel ");
	while (1) {
		/* Accept incomming connection request receive on socket */
		new_sock = accept( sock, (struct sockaddr *) &sock_addr, &len);
		if (new_sock < 0){
				if (errno == EINTR)
					continue;

				close_ipc_channel(sock);
				LOG_MSG(LOG_ERROR,"Socket connection accept failed error: %s", strerror(errno));
				/* Greacefull Exit */
				exit(0);
		} else {
            LOG_MSG(LOG_INIT, "accept_from_ipc_channel break ");
			break;
		}
	}

	LOG_MSG(LOG_INIT, "CP: Gx_app client connection succesfully accepted...!!!");

	return new_sock;
}

void
listen_ipc_channel( int sock )
{
	/* Mark the socket as a passive socket to accept incomming connections */
	if( listen(sock, 100) == -1){
		close_ipc_channel(sock);
		LOG_MSG(LOG_ERROR, "Socket Listen failed error: %s", strerror(errno));
		/* Greacefull Exit */
		exit(0);
	}
	LOG_MSG(LOG_INIT,"CP: Unix Server waiting for Gx_app client connection...");
}

void
get_peer_name(int sock, struct sockaddr_un sock_addr)
{
	socklen_t  len = sizeof(struct sockaddr_un);
	if( getpeername( sock, (struct sockaddr *) &sock_addr, &len) == -1) {
		if(errno != EINTR)
		{
			LOG_MSG(LOG_ERROR, "Socket getpeername failed error: %s",strerror(errno));
			close_ipc_channel(sock);
			/* Greacefull Exit */
			exit(0);
		}
	} else {
		LOG_MSG(LOG_ERROR, "CP: Gx_app client socket path %s...!!!",sock_addr.sun_path);
	}

}

int
recv_from_ipc_channel(int sock, char *buf)
{
	int bytes_recv = 0;
	bytes_recv = recv(sock, buf, BUFFSIZE, 0 ) ;
	if ( bytes_recv <= 0 ){
		if(errno != EINTR){
			LOG_MSG(LOG_ERROR, "Socket recv failed error: %s",strerror(errno));
           close_ipc_channel(sock);
           /* Greacefull Exit */
           exit(0);
		}
	}
	return bytes_recv;
}

void
send_to_ipc_channel(int sock, char *buf, int len)
{
	if( send(sock, buf, len, 0) <= 0){
		if(errno != EINTR){
			LOG_MSG(LOG_ERROR, "Socket send failed error: %s", strerror(errno));
			close_ipc_channel(sock);
			/* Greacefull Exit */
			exit(0);
		}
	}
}

void
close_ipc_channel(int sock)
{
	/* Closed unix socket */
	close(sock);
}
