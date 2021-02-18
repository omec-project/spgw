// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __BEARER_TABLES_H__
#define __BEARER_TABLES_H__
#include <iostream>
#include <unordered_map>

class bearerTables
{
    public:
        int add_teid_to_bearer_mapping(uint32_t teid, void *entry)
        {
            std::pair<uint32_t, void*> pair(teid, entry);
            bearer_by_fteid_hash.insert(pair);
            return 0;
        }

        void *find_bearer_from_teid(uint32_t teid)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = bearer_by_fteid_hash.find (teid);
            if (it == bearer_by_fteid_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_bearer_teid_mapping(uint32_t teid)
        {
            bearer_by_fteid_hash.erase(teid);
            return 0;
        }

    private:
        std::unordered_map<uint32_t,void*> bearer_by_fteid_hash;
};
#endif


