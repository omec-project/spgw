// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <iostream>
#include "spgw_config.h"
#include "spgw_config_struct.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cp_log.h"


spgwConfigStore *config;

bool 
compare_sub_rules(const sub_selection_rule_t *rule1, const sub_selection_rule_t *rule2)
{
    if(rule1->rule_priority < rule2->rule_priority)
        return true;
    return false;
}

spgw_config_profile_t* spgwConfig::parse_subscriber_profiles_cpp(const char *jsonFile)
{
    LOG_MSG(LOG_INIT,"parsing config in class function");
    FILE* fp = fopen(jsonFile , "r");
    if(fp == NULL){
        LOG_MSG(LOG_ERROR, "The json config file specified does not exists");
        return nullptr;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    fclose(fp);
    rapidjson::Document doc;
    doc.ParseStream(is);
    if(!doc.IsObject()) {
        LOG_MSG(LOG_ERROR,"Error parsing the json config file"); 
        return nullptr;
    }
    spgw_config_profile_t *config = spgwConfig::parse_json_doc(doc);
    return config;
}

spgw_config_profile_t*
spgwConfig::parse_json_doc(rapidjson::Document &doc)
{
    spgw_config_profile_t *temp_config = new (spgw_config_profile_t);
    spgwConfigStore *config_store = new (spgwConfigStore);
    temp_config->config = (void *)config_store;

    if(doc.HasMember("subscriber-selection-rules"))
    {
        for(uint32_t i=0; i< doc["subscriber-selection-rules"].Size();i++) 
        {
            sub_selection_rule_t *sub_rule = new (sub_selection_rule_t);
            LOG_MSG(LOG_INIT,"\tSubscriber selection rule %d ",i); 
            sub_rule->keys = nullptr;
            sub_rule->selected_apn_profile = nullptr;
            sub_rule->selected_user_plane_profile = nullptr;
            sub_rule->selected_qos_profile = nullptr;
            sub_rule->selected_access_profile[0] = nullptr; 
            sub_rule->selected_access_profile[1] = nullptr; 
            sub_rule->selected_access_profile[2] = nullptr; 
            sub_rule->selected_access_profile[3] = nullptr; 
            const rapidjson::Value& subRuleSection = doc["subscriber-selection-rules"][i];
            if(subRuleSection.HasMember("priority"))
            {
                sub_rule->rule_priority = subRuleSection["priority"].GetInt();
                LOG_MSG(LOG_INIT,"\t\tkeys has priority %d ",sub_rule->rule_priority);
            }
            if(subRuleSection.HasMember("keys"))
            {
                sub_selection_keys_t *key = new (sub_selection_keys_t); 
                key->imsi.is_valid = false;
                key->plmn.is_valid = false;
                key->apn.is_valid = false;
                sub_rule->keys = key;
                const rapidjson::Value& ruleKeys = subRuleSection["keys"];
                if(ruleKeys.HasMember("imsi-range") && ruleKeys["imsi-range"].IsObject())
                {
                    LOG_MSG(LOG_INIT, "\t\t\tkeys has imsi-range Object ");
                    const rapidjson::Value& imsiKeys = ruleKeys["imsi-range"];
                    if(imsiKeys.HasMember("from"))
                    {
                        LOG_MSG(LOG_INIT, "\t\t\t\tIMSI range from present %lu ",imsiKeys["from"].GetInt64());
                    }
                    if(imsiKeys.HasMember("to"))
                    {
                        LOG_MSG(LOG_INIT,"\t\t\t\tIMSI range to present %lu ",imsiKeys["to"].GetInt64());
                    }
                    key->imsi.is_valid = true;
                    key->imsi.from_imsi= imsiKeys["from"].GetInt64(); 
                    key->imsi.to_imsi= imsiKeys["to"].GetInt64(); 
                }
                if(ruleKeys.HasMember("serving-plmn") && ruleKeys["serving-plmn"].IsObject())
                {
                    LOG_MSG(LOG_INIT, "\t\t\tkeys has serving-plmn Object ");
                    const rapidjson::Value& plmnKeys = ruleKeys["serving-plmn"];
                    uint16_t mcc, mnc;
                    if(plmnKeys.HasMember("mcc"))
                    {
                        mcc = plmnKeys["mcc"].GetInt();
                        LOG_MSG(LOG_INIT,"\t\t\t\tmcc %u",mcc);
                    }
                    if(plmnKeys.HasMember("mnc"))
                    {
                        mnc = plmnKeys["mnc"].GetInt();
                        LOG_MSG(LOG_INIT,"\t\t\t\tmnc %d",mnc);
                    }
                    if(plmnKeys.HasMember("tac"))
                    {
                        LOG_MSG(LOG_INIT,"\t\t\t\ttac %d",plmnKeys["tac"].GetInt());
                    }
                    key->plmn.is_valid = true;
                    uint16_t mcc_dig_1, mcc_dig_2, mcc_dig_3;
                    uint16_t mnc_dig_1, mnc_dig_2, mnc_dig_3;
                    mcc_dig_1 = mcc/100; mcc = mcc % 100;
                    mcc_dig_2 = mcc/10; mcc = mcc %10;
                    mcc_dig_3 = mcc;
                    if(mnc > 99) 
                    {
                        mnc_dig_1 = (mnc/100);
                        mnc_dig_2 = (mnc%100)/10;
                        mnc_dig_3 = mnc % 10;
                    }
                    else
                    {
                        mnc_dig_3 = 0xf; 
                        mnc_dig_1 = mnc/10;
                        mnc_dig_2 = mnc % 10;
                    }

                    key->plmn.plmn[0] = (mcc_dig_2 << 4) | (mcc_dig_1); 
                    key->plmn.plmn[1] = (mnc_dig_1 << 4) | (mcc_dig_3);
                    key->plmn.plmn[2] = (mnc_dig_3 << 4) | (mnc_dig_2);
                    key->plmn.tac = plmnKeys["tac"].GetInt();
                }
                if(ruleKeys.HasMember("requested-apn"))
                {
                    const char *temp = ruleKeys["requested-apn"].GetString();
                    LOG_MSG(LOG_INIT,"\t\t\tkeys has requested-apn %s",temp);
                    key->apn.is_valid = true;
                    strcpy(key->apn.requested_apn,temp);
                    
                }
            }
            LOG_MSG(LOG_INIT,"\t\tSelected Profiles ");
            if(subRuleSection.HasMember("selected-apn-profile"))
            {
                const char *temp = subRuleSection["selected-apn-profile"].GetString();
                sub_rule->selected_apn_profile = (char *)malloc (strlen(temp)+1);
                LOG_MSG(LOG_INIT,"\t\t\tselected-apn-profile found - %s",temp);
                strcpy(sub_rule->selected_apn_profile, temp);
            }
            if(subRuleSection.HasMember("selected-user-plane-profile"))
            {
                const char *temp = subRuleSection["selected-user-plane-profile"].GetString();
                sub_rule->selected_user_plane_profile = (char *)malloc(strlen(temp)+1);
                LOG_MSG(LOG_INIT,"\t\t\tselected-user-plane-profile found %s",temp);
                strcpy(sub_rule->selected_user_plane_profile, temp);
            }
            if(subRuleSection.HasMember("selected-qos-profile"))
            {
                const char *temp = subRuleSection["selected-qos-profile"].GetString();
                sub_rule->selected_qos_profile = (char *)malloc(strlen(temp)+1);
                LOG_MSG(LOG_INIT,"\t\t\tselected-qos-profile found - %s",temp);
                strcpy(sub_rule->selected_qos_profile, temp);
            }
            if(subRuleSection.HasMember("selected-access-profile"))
            {
                for(uint32_t acc = 0; acc<subRuleSection["selected-access-profile"].Size() && (acc < 4) ; acc++)
                {
                    const rapidjson::Value& accessProfile= subRuleSection["selected-access-profile"][acc];
                    const char *temp = accessProfile.GetString(); 
                    sub_rule->selected_access_profile[acc] = (char *)malloc(strlen(temp)+1);
                    LOG_MSG(LOG_INIT,"\t\t\tselected-access-profile - %s",temp);
                    strcpy(sub_rule->selected_access_profile[acc], temp);
                }
            }            
            config_store->sub_sel_rules.push_back(sub_rule);
        }
        config_store->sub_sel_rules.sort(compare_sub_rules);
    }
    if(doc.HasMember("apn-profiles"))
    {
        const rapidjson::Value& apnProfileSection = doc["apn-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = apnProfileSection.MemberBegin(); itr != apnProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            if(itr->value.IsObject())
            {
                LOG_MSG(LOG_INIT,"\tAPN profile %s is Object", key.c_str());
                apn_profile_t *apn_profile = new (apn_profile_t);
                memset(apn_profile, 0, sizeof(apn_profile_t));
                strcpy(apn_profile->apn_profile_name, key.c_str());
                const rapidjson::Value& apnSection = itr->value; 
                if(apnSection.HasMember("apn-name")) {
                    const char *temp = apnSection["apn-name"].GetString();
                    LOG_MSG(LOG_INIT,"\t\tAPN name[%s]",temp);
                    /* Don't copy NULL termination */
                    strncpy(&apn_profile->apn_name[1], temp, strlen(temp));
                    char *ptr, *size;
                    size = &apn_profile->apn_name[0];
                    *size = 0;
                    ptr = apn_profile->apn_name + strlen(temp); // since we have added space at the start ptr will point to last char  
                    do {
                        if (ptr == size)
                            break;
                        if (*ptr == '.') {
                            *ptr = *size;
                            *size = 0;
                        } else {
                            (*size)++;
                        }
                        --ptr;
                    } while (ptr != apn_profile->apn_name);
                    apn_profile->apn_name_length = strlen(apn_profile->apn_name);
                    LOG_MSG(LOG_INIT,"\t\tAPN name after encode [%s]",apn_profile->apn_name);
                }
                if(apnSection.HasMember("usage")) {
                    int usage = apnSection["usage"].GetInt();
                    LOG_MSG(LOG_INIT,"\t\tUsage type  %d ",usage);
                    apn_profile->apn_usage_type = usage;
                }
                if(apnSection.HasMember("network")) {
                    const char *temp = apnSection["network"].GetString();
                    LOG_MSG(LOG_INIT,"\t\tNetwork type %s",temp);;
                    strcpy(apn_profile->apn_net_cap, temp);
                }
                if(apnSection.HasMember("gx_enabled")) {
                    bool gx_enabled = apnSection["gx_enabled"].GetBool();
                    LOG_MSG(LOG_INIT,"\t\tGx enabled %s",gx_enabled == true ? "True":"False");
                    apn_profile->gx_enabled = gx_enabled;
                }
                if(apnSection.HasMember("mtu")) {
                    uint16_t mtu = apnSection["mtu"].GetInt();
                    LOG_MSG(LOG_INIT,"\t\tAPN has mtu %u",mtu);
                    apn_profile->mtu = mtu;
                }
                if(apnSection.HasMember("dns_primary")) {
                    const char *temp = apnSection["dns_primary"].GetString();
                    LOG_MSG(LOG_INIT,"\t\tAPN has dns_primary %s ",temp);
                    struct in_addr temp_i;
                    inet_aton(temp, &temp_i);
                    apn_profile->dns_primary = temp_i.s_addr;
                }
                if(apnSection.HasMember("dns_secondary")) {
                    const char *temp = apnSection["dns_secondary"].GetString();
                    LOG_MSG(LOG_INIT,"\t\tAPN has dns_secondary %s",temp);
                    struct in_addr temp_i;
                    inet_aton(temp, &temp_i);
                    apn_profile->dns_secondary = temp_i.s_addr;
                }
                config_store->apn_profile_list.push_back(apn_profile);
            }
        }
    }
     
    if(doc.HasMember("user-plane-profiles"))
    {
        const rapidjson::Value& userProfileSection = doc["user-plane-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = userProfileSection.MemberBegin(); itr != userProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            user_plane_profile_t *user_plane = new (user_plane_profile_t);
            user_plane->global_address = true; // default global addressing mode
            user_plane->upf_addr = 0;
            const rapidjson::Value& userPlaneSection = itr->value; 
            strcpy(user_plane->user_plane_profile_name, key.c_str());
            if(userPlaneSection.HasMember("user-plane"))
            {
                const char *temp = userPlaneSection["user-plane"].GetString();
                LOG_MSG(LOG_INIT,"\tUser Plane - %s ",temp);
                strcpy(user_plane->user_plane_service, temp);
            }
            if(userPlaneSection.HasMember("global-address"))
            {
                user_plane->global_address = userPlaneSection["global-address"].GetBool();
            }
            if(userPlaneSection.HasMember("qos-tags"))
            {
                LOG_MSG(LOG_INIT,"\t\tQoS Tags specified "); 
            }
            if(userPlaneSection.HasMember("access-tags"))
            {
                LOG_MSG(LOG_INIT,"\t\tAccess Tags specified ");
            }
            config_store->user_plane_list.push_back(user_plane);
        }
    }
    if(doc.HasMember("qos-profiles"))
    {
        const rapidjson::Value& qosProfileSection = doc["qos-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = qosProfileSection.MemberBegin(); itr != qosProfileSection.MemberEnd(); ++itr)
        {
            qos_profile_t *qos_profile = new (qos_profile_t);
            std::string key = itr->name.GetString();
            strcpy(qos_profile->qos_profile_name, key.c_str());
            LOG_MSG(LOG_INIT,"\tQoS profile - %s",key.c_str());
            const rapidjson::Value& qosPlaneSection = itr->value; 
            qos_profile->apn_ambr_ul = qosPlaneSection["apn-ambr"][0].GetInt64();
            qos_profile->apn_ambr_dl = qosPlaneSection["apn-ambr"][1].GetInt64();
            LOG_MSG(LOG_INIT,"\t\tQoS apn ambr uplink - %u, downlink %u ",qos_profile->apn_ambr_ul, qos_profile->apn_ambr_dl);
            config_store->qos_profile_list.push_back(qos_profile);
        }
    }
    if(doc.HasMember("access-profiles"))
    {
        const rapidjson::Value& accessProfileSection = doc["access-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = accessProfileSection.MemberBegin(); itr != accessProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            LOG_MSG(LOG_INIT,"\tAccess profile - %s",key.c_str());
            access_profile_t *access_profile = new (access_profile_t);
            strcpy(access_profile->access_profile_name, key.c_str());
            config_store->access_profile_list.push_back(access_profile);
        }
    }    
    return temp_config;
}

sub_profile_t* 
spgwConfig::match_sub_selection_cpp(sub_selection_keys_t *key)
{
    sub_selection_rule_t *rule = nullptr;
    std::list<sub_selection_rule_t *>::iterator it;
    if(config == NULL) {
        LOG_MSG(LOG_ERROR,"Subscriber mapping configuration not available...");
        return NULL;
    }
    for (it=config->sub_sel_rules.begin(); it!=config->sub_sel_rules.end(); ++it)
    {
        rule = *it;
        LOG_MSG(LOG_DEBUG,"Searching rule %d ", rule->rule_priority);
        sub_selection_keys_t *key_l = rule->keys;
        if((key_l != nullptr) && (key_l->imsi.is_valid))
        {
            if(key->imsi.is_valid == false)
            {
                continue; // no match continue for next rule 
            }
            if(!((key->imsi.from_imsi >= key_l->imsi.from_imsi) && (key->imsi.from_imsi <= key_l->imsi.to_imsi)))
            {
               LOG_MSG(LOG_DEBUG, "IMSI range not matched");
               continue; // no match continue for next rule  
            }
        }
        LOG_MSG(LOG_DEBUG, "IMSI range matched for %lu", key->imsi.from_imsi);
        if((key_l != nullptr) && (key_l->plmn.is_valid))
        {
            if(key->plmn.is_valid == false)
                continue; // no match . Continue for next rule 
            if(memcmp(&key_l->plmn.plmn[0], &key_l->plmn.plmn[0],3))
                continue; // no match 
            if(key_l->plmn.tac != key->plmn.tac)
            {
                LOG_MSG(LOG_DEBUG, "Subscriber not matched with PLMN ");
                continue; // no match 
            }
        } 
        LOG_MSG(LOG_DEBUG, "Subscriber matched with PLMN ");
        if((key_l != nullptr) && (key_l->apn.is_valid))
        {
            if(key->apn.is_valid == false)
                continue;
            if(strcmp(key->apn.requested_apn, key_l->apn.requested_apn))
            {
                LOG_MSG(LOG_DEBUG, "Subscriber not matched with APN");
                continue;
            }
        }
        LOG_MSG(LOG_DEBUG, "Subscriber matched with APN");
        break;
    }

    if(it != config->sub_sel_rules.end())
    {
        // We reached here means we have matching rule 
        sub_profile_t *temp = new (sub_profile_t);
        temp->apn_profile = config->get_apn_profile(rule->selected_apn_profile);
        temp->qos_profile = config->get_qos_profile(rule->selected_qos_profile);
        temp->up_profile = config->get_user_plane_profile(rule->selected_user_plane_profile);
        temp->access_profile = config->get_access_profile(rule->selected_access_profile[0]);
        LOG_MSG(LOG_DEBUG,"matching subscriber rule found ");
        return temp;
    }
    LOG_MSG(LOG_INIT,"No matching rule found ");
    return nullptr;
}

// get config reference in global variable
spgwConfigStore* spgwConfig::get_cp_config_cpp()
{
    return config;
}

// set config reference in global variable 
void spgwConfig::set_cp_config_cpp(spgw_config_profile_t *new_config)
{
    config = reinterpret_cast<spgwConfigStore *>(new_config->config); 
}

void spgwConfig::switch_config_cpp(spgw_config_profile_t *new_config)
{
    spgwConfigStore *temp = config; // take old config pointer reference 

    config = reinterpret_cast<spgwConfigStore *>(new_config->config); 

    // release old config
     delete temp;
}

apn_profile_t* spgwConfig::match_apn_profile_cpp(char *name, uint16_t len)
{
    std::list<apn_profile_t*>::iterator it;
    LOG_MSG(LOG_DEBUG,"Search APN [%s] in APN profile list", name);
    for (it=config->apn_profile_list.begin(); it!=config->apn_profile_list.end(); ++it)
    {
        apn_profile_t* apn = *it;
        LOG_MSG(LOG_DEBUG, "Compare APN profile [%s] with [%s] ", apn->apn_name, name);
        if(strlen(apn->apn_name) != len)
            continue;
        if(memcmp(apn->apn_name,name,len) != 0)
            continue;
        LOG_MSG(LOG_DEBUG,"Found APN profile   %s ", apn->apn_name);
        return apn;
    }
    LOG_MSG(LOG_DEBUG,"APN [%s] not found ", name);
    return nullptr;
}

void 
spgwConfig::invalidate_user_plane_address(uint32_t addr) 
{
    for (std::list<user_plane_profile_t*>::iterator it=config->user_plane_list.begin(); it!=config->user_plane_list.end(); ++it)
    {
        user_plane_profile_t *up=*it;
        if(up->upf_addr == addr)
        {
            LOG_MSG(LOG_INIT, "invalidating upf address");
            up->upf_addr = 0;
        }
    }
    return ;
}
