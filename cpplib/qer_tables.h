// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __QER_TABLES_H__
#define __QER_TABLES_H__

#include <iostream>
#include <unordered_map>

class qerTables
{
    public:
        int add_qerid_to_qerctxt_mapping(uint32_t qer_id, void *entry)
        {
            std::pair<uint32_t, void*> pair(qer_id, entry);
            qerid_qerctxt_hash.insert(pair);
            return 0;
        }

        void *find_qerctxt_from_qerid(uint32_t qer_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = qerid_qerctxt_hash.find (qer_id);
            if (it == qerid_qerctxt_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_qerid_qerctxt_mapping(uint32_t qer_id)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = qerid_qerctxt_hash.find (qer_id);
            if (it == qerid_qerctxt_hash.end()) {
                return -1;
            }
            void *temp = it->second;
            qerid_qerctxt_hash.erase(it);
            free(temp);
            return 0;
        }
    private:
        std::unordered_map<uint32_t,void*> qerid_qerctxt_hash;
};
#endif
