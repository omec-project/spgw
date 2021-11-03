// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __UPF_TABLES_H__
#define __UPF_TABLES_H__
#include <iostream>
#include <unordered_map>
#include <sstream>

class upfTables
{
    public:
        int add_upf_service(const char *upf_service, void *entry)
        {
            std::cout<<"adding upf service entry "<<upf_service<<std::endl;
            std::stringstream service_s;
            service_s<<upf_service;
            std::pair<std::string, void*> pair(service_s.str(), entry);
            upf_context_by_service_hash.insert(pair);
            return 0;
        }
        void *find_upf_service(const char *upf_service)
        {
            std::stringstream service_s;
            service_s<<upf_service;
            std::unordered_map<std::string, void*>::const_iterator it = upf_context_by_service_hash.find (service_s.str());
            if (it == upf_context_by_service_hash.end()) {
                std::cout<<"Not Found upf service entry ["<<upf_service<<"] in table"<<std::endl;
                return NULL;
            }
            return it->second;
        }

        int delete_upf_service(const char *upf_service)
        {
            std::stringstream service_s;
            service_s<<upf_service;
            std::cout<<"delete upf service entry ["<<upf_service<<"] from table"<<std::endl;
            upf_context_by_service_hash.erase(service_s.str());
            return 0;
        }

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
        std::unordered_map<std::string,void*> upf_context_by_service_hash;
};
#endif


