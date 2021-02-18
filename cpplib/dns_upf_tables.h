// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __DNS_UPF_TABLES_H__
#define __DNS_UPF_TABLES_H__
#include <iostream>
#include <unordered_map>

class dnsUpfTables
{
    public:
        int add_imsi_to_upfdns_mapping(uint64_t imsi, void *entry)
        {
            std::pair<uint64_t, void*> pair(imsi, entry);
            imsi_dnsupf_hash.insert(pair);
            return 0;
        }

        void *find_upfdns_from_imsi(uint64_t imsi)
        {
            std::unordered_map<uint64_t, void*>::const_iterator it = imsi_dnsupf_hash.find (imsi);
            if (it == imsi_dnsupf_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_imsi_upfdns_mapping(uint64_t imsi)
        {
            imsi_dnsupf_hash.erase(imsi);
            return 0;
        }
    private:
        std::unordered_map<uint64_t,void*> imsi_dnsupf_hash;
};
#endif
