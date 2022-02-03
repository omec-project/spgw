// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __URR_TABLES_H__
#define __URR_TABLES_H__
#include <iostream>
#include <unordered_map>

class urrTables
{
    public:
        int add_urrid_to_urrctxt_mapping(uint32_t urr_id, void *entry)
        {
            std::pair<uint32_t, void*> pair(urr_id, entry);
            urrid_urrctxt_hash.insert(pair);
            return 0;
        }

        void *find_urrctxt_from_urrid(uint32_t urr_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = urrid_urrctxt_hash.find (urr_id);
            if (it == urrid_urrctxt_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_urrid_urrctxt_mapping(uint32_t urr_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = urrid_urrctxt_hash.find (urr_id);
            if (it == urrid_urrctxt_hash.end()) {
                return -1;
            }
            void *temp = it->second;
            urrid_urrctxt_hash.erase(it);
            free(temp);
            return 0;
        }
    private:
        std::unordered_map<uint32_t,void*> urrid_urrctxt_hash;
};
#endif
