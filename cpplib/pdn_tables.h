// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PDN_TABLES_H__
#define __PDN_TABLES_H__
#include <iostream>
#include <unordered_map>

class pdnTables
{
    public:
        int add_callid_to_pdn_mapping(uint32_t call_id, void *entry)
        {
            std::pair<uint32_t, void*> pair(call_id, entry);
            pdn_by_callid_hash.insert(pair);
            return 0;
        }

        void *find_pdn_from_callid(uint32_t call_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = pdn_by_callid_hash.find (call_id);
            if (it == pdn_by_callid_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_callid_pdn_mapping(uint32_t call_id)
        {
            pdn_by_callid_hash.erase(call_id);
            return 0;
        }

        int add_teid_to_pdn_mapping(uint32_t teid, void *entry)
        {
            std::pair<uint32_t, void*> pair(teid, entry);
            pdn_by_fteid_hash.insert(pair);
            return 0;
        }

        void *find_pdn_from_teid(uint32_t teid)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = pdn_by_fteid_hash.find (teid);
            if (it == pdn_by_fteid_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_teid_pdn_mapping(uint32_t teid)
        {
            pdn_by_fteid_hash.erase(teid);
            return 0;
        }

    private:
        std::unordered_map<uint32_t,void*> pdn_by_callid_hash;
        std::unordered_map<uint32_t,void*> pdn_by_fteid_hash;
};
#endif
