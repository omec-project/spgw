// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#ifndef __SPGW_CONFIG_H__
#define __SPGW_CONFIG_H__
#include "spgw_config_struct.h"
#include <list>
#include "string.h"
#include "stdlib.h"

class spgwConfigStore
{
    public:
        spgwConfigStore() {;}
        ~spgwConfigStore()
        {
            for (std::list<sub_selection_rule_t *> ::iterator it=sub_sel_rules.begin(); it!=sub_sel_rules.end(); ++it)
            {
                sub_selection_rule_t *rule = *it;
                delete(rule->keys);
                free(rule->selected_apn_profile);
                free(rule->selected_user_plane_profile);
                free(rule->selected_qos_profile);
                free(rule->selected_access_profile[0]);
                free(rule->selected_access_profile[1]);
                free(rule->selected_access_profile[2]);
                free(rule->selected_access_profile[3]);
                delete rule;
            }

            for (std::list<apn_profile_t*>::iterator it=apn_profile_list.begin(); it!=apn_profile_list.end(); ++it)
            {
                apn_profile_t *apn=*it;
                delete apn;
            }
            for (std::list<user_plane_profile_t*>::iterator it=user_plane_list.begin(); it!=user_plane_list.end(); ++it)
            {
                user_plane_profile_t *up=*it;
                delete up;
            }
            for (std::list<qos_profile_t*>::iterator it=qos_profile_list.begin(); it!=qos_profile_list.end(); ++it)
            {
                qos_profile_t *qos=*it;
                delete qos;
            }
            for (std::list<access_profile_t*>::iterator it=access_profile_list.begin(); it!=access_profile_list.end(); ++it)
            {
                access_profile_t *access=*it;
                delete access;
            }
        }

        apn_profile_t* get_apn_profile(char *name) 
        {
            for (std::list<apn_profile_t*>::iterator it=apn_profile_list.begin(); it!=apn_profile_list.end(); ++it)
            {
                apn_profile_t *apn=*it;
                if(strcmp(apn->apn_profile_name, name) == 0)
                    return apn; 
            }
            return nullptr;
        }

        user_plane_profile_t* get_user_plane_profile(char *name) 
        {
            for (std::list<user_plane_profile_t*>::iterator it=user_plane_list.begin(); it!=user_plane_list.end(); ++it)
            {
                user_plane_profile_t *up=*it;
                if(strcmp(up->user_plane_profile_name, name) == 0)
                    return up; 
            }
            return nullptr;
        }
        qos_profile_t* get_qos_profile(char *name) 
        {
            for (std::list<qos_profile_t*>::iterator it=qos_profile_list.begin(); it!=qos_profile_list.end(); ++it)
            {
                qos_profile_t *qos=*it;
                if(strcmp(qos->qos_profile_name, name) == 0)
                    return qos; 
            }
            return nullptr;
        }
        access_profile_t* get_access_profile(char *name) 
        {
            for (std::list<access_profile_t*>::iterator it=access_profile_list.begin(); it!=access_profile_list.end(); ++it)
            {
                access_profile_t *access=*it;
                if(strcmp(access->access_profile_name, name) == 0)
                    return access; 
            }
            return nullptr;
        }
    public:
        std::list<sub_selection_rule_t *> sub_sel_rules;
        std::list<apn_profile_t *> apn_profile_list;
        std::list<user_plane_profile_t*> user_plane_list;
        std::list<access_profile_t*> access_profile_list;
        std::list<qos_profile_t*> qos_profile_list;
};

class spgwConfig
{
   public:
    static spgw_config_profile_t *parse_subscriber_profiles_cpp(const char *); 
    static void set_cp_config_cpp(spgw_config_profile_t *);
    static void switch_config_cpp(spgw_config_profile_t *);
    static sub_profile_t* match_sub_selection_cpp(sub_selection_keys_t *key);
    static apn_profile_t * match_apn_profile_cpp(char *, uint16_t len);
};
#endif
