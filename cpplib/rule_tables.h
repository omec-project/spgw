// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __RULE_TABLES_H__
#define __RULE_TABLES_H__

#include <iostream>
#include <unordered_map>
#include <sstream>

class ruleTables
{
    public:
        int add_rulename_to_bearerid_mapping(const char *rule_name, uint8_t bearerid)
        {
            std::stringstream rule_s;
            rule_s<<rule_name;
            std::pair<std::string, uint8_t> pair(rule_s.str(), bearerid);
            // should we override ??
            rulename_bearerid_hash.insert(pair);
            return 0;
        }

        int8_t find_bearerid_from_rulename(const char *rule_name)
        {
            std::stringstream rule_s;
            rule_s<<rule_name;
            std::unordered_map<std::string, uint8_t>::const_iterator it = rulename_bearerid_hash.find (rule_s.str());
            if (it == rulename_bearerid_hash.end()) {
                return -1;
            }
            return it->second;
        }

        int delete_bearerid_rulename_mapping(const char *rule_name)
        {
            std::stringstream rule_s;
            rule_s<<rule_name;
            rulename_bearerid_hash.erase(rule_s.str());
            return 0;
        }
    private:
        std::unordered_map<std::string,uint8_t> rulename_bearerid_hash;
};
#endif
