// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2017 Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only

#ifndef _SSL_CLIENT_H_
#define _SSL_CLIENT_H_

#include <openssl/ssl.h>

#define SSL_CONN_FAIL -1

/**
 * @brief  : Initialize ssl connection and verify SGX's parameter from certificate
 * @param  : hostname, hostname or ip of remote host.
 * @param  : portnum, port number of remote host.
 * @param  : client_cert_path, path of client certificate.
 * @param  : priv_key_path, path of private key.
 * @param  : mrenclave, mrenclave value read from file
 * @param  : mrsigner, mrsigner value read from file
 * @param  : isvsvn, isvsvn value read from file
 * @return : Returns ssl handle on success, -1 otherwise
 */
SSL *
sgx_cdr_channel_init(const char *hostname, const char *portnum,
			const char *client_cert_path, const char *priv_key_path,
			const char *mrenclave, const char *mrsigner, const char *isvsvn);

/**
 * @brief  : Free ssl context and close the connection
 * @param  : ssl, ssl handle.
 * @return : Return nothing
 */
void
sgx_cdr_channel_close(SSL *ssl);

#endif /* _SSL_CLIENT_H_ */

