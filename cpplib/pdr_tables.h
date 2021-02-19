// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __PDR_TABLES_H__
#define __PDR_TABLES_H__

#include <iostream>
#include <unordered_map>

class pdrTables
{
    public:
        int add_pdrid_to_pdrctxt_mapping(uint32_t pdr_id, void *entry)
        {
            std::pair<uint32_t, void*> pair(pdr_id, entry);
            pdrid_pdrctxt_hash.insert(pair);
            return 0;
        }

        void *find_pdrctxt_from_pdrid(uint32_t pdr_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = pdrid_pdrctxt_hash.find (pdr_id);
            if (it == pdrid_pdrctxt_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_pdrid_pdrctxt_mapping(uint32_t pdr_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = pdrid_pdrctxt_hash.find (pdr_id);
            if (it == pdrid_pdrctxt_hash.end()) {
                return -1;
            }
            void *temp = it->second;
            pdrid_pdrctxt_hash.erase(it);
            free(temp);
            return 0;
        }
    private:
        std::unordered_map<uint32_t,void*> pdrid_pdrctxt_hash;
};
#endif
