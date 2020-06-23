// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#define _GNU_SOURCE     /* Expose declaration of tdestroy() */
#include "util.h"
#include "up_acl.h"

extern struct rte_hash *sess_ctx_by_sessid_hash;
extern struct rte_hash *sess_by_teid_hash;
extern struct rte_hash *sess_by_ueip_hash;
extern struct rte_hash *pdr_by_id_hash;
extern struct rte_hash *far_by_id_hash;
extern struct rte_hash *qer_by_id_hash;
extern struct rte_hash *urr_by_id_hash;


/* Retrive the Session information based on teid */
int
iface_lookup_uplink_data(struct ul_bm_key *key,
		void **value)
{
	return rte_hash_lookup_data(sess_by_teid_hash, key, value);
}

/* Retrive the Session information based on teid */
int
iface_lookup_uplink_bulk_data(const void **key, uint32_t n,
		uint64_t *hit_mask, void **value)
{
	return rte_hash_lookup_bulk_data(sess_by_teid_hash, key, n, hit_mask, value);
}

/* Retrive the Session information based on UE IP */
int
iface_lookup_downlink_data(struct dl_bm_key *key,
		void **value)
{
	return rte_hash_lookup_data(sess_by_ueip_hash, key, value);
}

/* Retrive the Session information based on UE IP */
int
iface_lookup_downlink_bulk_data(const void **key, uint32_t n,
		uint64_t *hit_mask, void **value)
{
	return rte_hash_lookup_bulk_data(sess_by_ueip_hash, key, n, hit_mask, value);
}


