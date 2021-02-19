// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PEER_TABLES_H__
#define __PEER_TABLES_H__
#include <iostream>
#include <unordered_map>

class peerRecord
{
    public:
        peerRecord()
        {
            peer_rec_m = NULL; 
            recov_time_m = 0;
        }
        ~peerRecord()
        {
        }

        void set_peer_private_data(void *peer_rec) 
        {
            peer_rec_m = peer_rec;
            return;
        }
        void *get_peer_private_data()
        {
            return peer_rec_m;
        }
        void set_recov_time(uint32_t time)
        {
            recov_time_m = time;
        }
        uint32_t get_recov_time()
        {
            return recov_time_m;
        }
    private:
        uint32_t recov_time_m;
        void     *peer_rec_m;
};

class peerTables
{
    public:
        int add_addr_to_peer_mapping(uint32_t peer_addr, void *entry)
        {
            peerRecord *peer = new peerRecord();
            peer->set_peer_private_data(entry);
            std::pair<uint32_t, peerRecord*> pair(peer_addr, peer);
            addr_to_peer_context_hash.insert(pair);
            return 0;
        }

        void *find_peer_from_addr(uint32_t peer_addr)
        {
            std::unordered_map<uint32_t, peerRecord*>::const_iterator it = addr_to_peer_context_hash.find (peer_addr);
            if (it == addr_to_peer_context_hash.end()) {
                return NULL;
            }
            peerRecord *peer = it->second;
            return peer->get_peer_private_data();
        }

        int delete_addr_peer_mapping(uint32_t peer_addr)
        {
            addr_to_peer_context_hash.erase(peer_addr);
            return 0;
        }

        int add_peer_addr_to_recov_time_mapping(uint32_t peer_addr, uint32_t recov_time)
        {
            peerRecord *peer;
            std::unordered_map<uint32_t, peerRecord*>::const_iterator it = addr_to_peer_context_hash.find (peer_addr);
            if (it == addr_to_peer_context_hash.end()) {
                return -1;
            }
            peer = it->second;
            peer->set_recov_time(recov_time);
            return 0;
        }

        int find_peer_recov_time(uint32_t peer_addr, uint32_t *recov_time)
        {
            std::unordered_map<uint32_t, peerRecord*>::const_iterator it = addr_to_peer_context_hash.find (peer_addr);
            if (it == addr_to_peer_context_hash.end()) {
                return -1;
            }
            peerRecord *peer = it->second;
            *recov_time = peer->get_recov_time();
            return 0;
        }

        int reset_peer_recov_time(uint32_t peer_addr)
        {
            peerRecord *peer;
            std::unordered_map<uint32_t, peerRecord*>::const_iterator it = addr_to_peer_context_hash.find (peer_addr);
            if (it == addr_to_peer_context_hash.end()) {
                return -1;
            }
            peer = it->second;
            peer->set_recov_time(0);
            return 0;
        }

    private:
        std::unordered_map<uint32_t,peerRecord*> addr_to_peer_context_hash;
};
#endif
