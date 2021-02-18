// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __UPF_TABLES_H__
#define __UPF_TABLES_H__
#include <iostream>
#include <unordered_map>

class upfTables
{
    public:
        int add_upf(uint32_t *upf_ip, void *entry)
        {
            std::pair<uint32_t, void*> pair(*upf_ip, entry);
            upf_context_by_ip_hash.insert(pair);
            return 0;
        }
        void *find_upf(uint32_t upf_ip)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = upf_context_by_ip_hash.find (upf_ip);
            if (it == upf_context_by_ip_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_upf(uint32_t upf_ip)
        {
            upf_context_by_ip_hash.erase(upf_ip);
            return 0;
        }

    private:
        std::unordered_map<uint32_t,void*> upf_context_by_ip_hash;
};
#endif


