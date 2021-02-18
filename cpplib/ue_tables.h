// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __UE_TABLES_H__
#define __UE_TABLES_H__
#include <iostream>
#include <unordered_map>
#include <sstream>

class ueTables
{
    public:
        int add_imsi_to_ue_mapping(uint64_t imsi, void *entry)
        {
            std::pair<uint64_t, void*> pair(imsi, entry);
            imsi_session_hash.insert(pair);
            return 0;
        }

        void *find_ue_from_imsi(uint64_t imsi)
        {
            std::unordered_map<uint64_t, void*>::const_iterator it = imsi_session_hash.find (imsi);
            if (it == imsi_session_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_imsi_ue_mapping(uint64_t imsi)
        {
            imsi_session_hash.erase(imsi);
            return 0;
        }



        int add_seid_to_ue_mapping(uint64_t seid, void *entry)
        {
            std::pair<uint64_t, void*> pair(seid, entry);
            seid_session_hash.insert(pair);
            return 0;
        }

        void *find_ue_from_seid(uint64_t seid)
        {
            std::unordered_map<uint64_t, void*>::const_iterator it = seid_session_hash.find (seid);
            if (it == seid_session_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_seid_ue_mapping(uint64_t seid)
        {
            seid_session_hash.erase(seid);
            return 0;
        }

        int add_teid_to_ue_mapping(uint32_t teid, void *entry)
        {
            std::pair<uint32_t, void*> pair(teid, entry);
            teid_session_hash.insert(pair);
            return 0;
        }

        void *find_ue_from_teid(uint32_t teid)
        {
            std::unordered_map<uint32_t, void*>::const_iterator it = teid_session_hash.find (teid);
            if (it == teid_session_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_teid_ue_mapping(uint32_t teid)
        {
            teid_session_hash.erase(teid);
            return 0;
        }

        int add_gx_sessid_to_ue_mapping(const uint8_t *sess_id, void *entry)
        {
            std::stringstream sessid_s;
            sessid_s<<sess_id;
            std::pair<std::string, void*> pair(sessid_s.str(), entry);
            gx_sessid_session_hash.insert(pair);
            return 0;
        }

        void *find_ue_from_gx_sessid(const uint8_t *sess_id)
        {
            std::stringstream sessid_s;
            sessid_s<<sess_id;
            std::unordered_map<std::string, void*>::const_iterator it = gx_sessid_session_hash.find (sessid_s.str());
            if (it == gx_sessid_session_hash.end()) {
                return NULL;
            }
            return it->second;
        }

        int delete_gx_sessid_ue_mapping(const uint8_t *sess_id)
        {
            std::stringstream sessid_s;
            sessid_s<<sess_id;
            gx_sessid_session_hash.erase(sessid_s.str());
            return 0;
        }
    private:
        std::unordered_map<uint32_t,void*> teid_session_hash;
        std::unordered_map<uint64_t,void*> seid_session_hash;
        std::unordered_map<uint64_t,void*> imsi_session_hash;
        std::unordered_map<std::string, void*> gx_sessid_session_hash;
};
#endif
