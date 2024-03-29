// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include "spgw_config.h"
#include "spgw_cpp_wrapper.h"
#include "spgw_config_struct.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cp_log.h"
#include <mutex>
#include "ip_pool_mgmt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

spgwConfigStore *sub_classifier_config;
std::mutex config_mtx; 

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

void clearOldUpfProfiles(spgwConfigStore* oldCfg, spgwConfigStore* newCfg) {
    LOG_MSG(LOG_DEBUG, "clearOldUpfProfiles");
    bool found;
    for (std::list<user_plane_profile_t*>::iterator it=oldCfg->user_plane_list.begin(); it!=oldCfg->user_plane_list.end(); ++it)
    {
        user_plane_profile_t *upOld=*it;
        found = false;
        for (std::list<user_plane_profile_t*>::iterator it2=newCfg->user_plane_list.begin(); it2!=newCfg->user_plane_list.end(); ++it2) {
            user_plane_profile_t *upNew=*it2;
            if (strcmp(upOld->user_plane_profile_name, upNew->user_plane_profile_name) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            LOG_MSG(LOG_DEBUG, "upf deleted in new config: %s", upOld->user_plane_profile_name);
            cfg_func_ptr func = getConfigCallback();
            if (func != NULL) {
                config_callback_t *conf_struct = (config_callback_t *)calloc(1, sizeof(config_callback_t));
                if (conf_struct == NULL) {
                    LOG_MSG(LOG_ERROR, "Config callback struct allocation failed.");
                    return;
                }
                conf_struct->action = DISABLE_UPF;
                strcpy(conf_struct->upf_service_name, upOld->user_plane_service);
                func(conf_struct);
            }
        }
    }
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
                    strcpy(&key->apn.requested_apn[1], temp);
                    char *ptr, *size;
                    size = &key->apn.requested_apn[0];
                    *size = 0;
                    // since we have added space at the start ptr will point to last char
                    ptr = key->apn.requested_apn + strlen(temp); 
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
                    } while (ptr != key->apn.requested_apn);
                    LOG_MSG(LOG_INIT,"\t\tAPN name after encode [%s] and length %lu ",key->apn.requested_apn, strlen(key->apn.requested_apn));

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
            const rapidjson::Value& qosSection = itr->value;
            qos_profile_t *qos_profile = new (qos_profile_t);
            std::string key = itr->name.GetString();
            strcpy(qos_profile->qos_profile_name, key.c_str());
            LOG_MSG(LOG_INIT,"\tQoS profile - %s",key.c_str());
            const rapidjson::Value& qosPlaneSection = itr->value; 
            qos_profile->apn_ambr_ul = qosPlaneSection["apn-ambr"][0].GetInt64();
            qos_profile->apn_ambr_dl = qosPlaneSection["apn-ambr"][1].GetInt64();
            if(qosSection.HasMember("qci")) {
                qos_profile->qci = qosSection["qci"].GetUint();
            } else {
                qos_profile->qci = 9;
            }
//            if(qosSection.HasMember("arp")) {
//                qos_profile->arp = qosSection["arp"].GetUint();
//            } else {
                qos_profile->arp = 1;
//            }

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
    
    std::unique_lock<std::mutex> lock(config_mtx);
    spgwConfigStore *temp = sub_classifier_config;
    if (temp != NULL) {
        clearOldUpfProfiles(temp, config_store);
    }
    sub_classifier_config = config_store; // move to new config
    delete temp;
    return temp_config;
}

sub_config_t *
spgwConfig::match_sub_selection_cpp(sub_selection_keys_t *key)
{
    std::unique_lock<std::mutex> lock(config_mtx);
    sub_selection_rule_t *rule = nullptr;
    std::list<sub_selection_rule_t *>::iterator it;
    if(sub_classifier_config == NULL) {
        LOG_MSG(LOG_ERROR,"Subscriber mapping configuration not available...");
        return NULL;
    }
    for (it = sub_classifier_config->sub_sel_rules.begin(); it != sub_classifier_config->sub_sel_rules.end(); ++it)
    {
        rule = *it;
        sub_selection_keys_t *key_l = rule->keys;
        if((key_l != nullptr) && (key_l->imsi.is_valid))
        {
            if(key->imsi.is_valid == false)
            {
                continue; // no match continue for next rule 
            }
            if(!((key->imsi.from_imsi >= key_l->imsi.from_imsi) && (key->imsi.from_imsi <= key_l->imsi.to_imsi)))
            {
               continue; // no match continue for next rule  
            }
        }
        if((key_l != nullptr) && (key_l->plmn.is_valid))
        {
            if(key->plmn.is_valid == false)
                continue; // no match . Continue for next rule 
            if(memcmp(&key_l->plmn.plmn[0], &key_l->plmn.plmn[0],3))
                continue; // no match 
            if(key_l->plmn.tac != key->plmn.tac)
            {
                continue; // no match 
            }
        } 
        if((key_l != nullptr) && (key_l->apn.is_valid))
        {
            if(key->apn.is_valid == false)
                continue;
            if(strcmp(key->apn.requested_apn, key_l->apn.requested_apn))
            {
                continue;
            }
        }
        break;
    }

    if(it != sub_classifier_config->sub_sel_rules.end())
    {
        // We reached here means we have matching rule 
        sub_config_t *temp = (sub_config_t *) calloc (1, sizeof(sub_config_t));
        apn_profile_t *apn_profile = sub_classifier_config->get_apn_profile(rule->selected_apn_profile);
        temp->dns_primary = apn_profile->dns_primary;
        temp->dns_secondary = apn_profile->dns_secondary;
        temp->mtu = apn_profile->mtu;

        user_plane_profile_t *up_profile =  sub_classifier_config->get_user_plane_profile(rule->selected_user_plane_profile);
        strcpy(temp->user_plane_service, up_profile->user_plane_service);
        temp->global_address = up_profile->global_address;
        qos_profile_t *qos_profile = sub_classifier_config->get_qos_profile(rule->selected_qos_profile);

        temp->apn_ambr_ul = qos_profile->apn_ambr_ul;
        temp->apn_ambr_dl = qos_profile->apn_ambr_dl;
        temp->arp = qos_profile->arp;
        temp->qci = qos_profile->qci;

        LOG_MSG(LOG_DEBUG,"matching subscriber rule found - User plane profile %s, UPF service %s ", rule->selected_user_plane_profile, up_profile->user_plane_service);
        return temp;
    }
    LOG_MSG(LOG_INIT,"No matching rule found for subscriber %lu ",key->imsi.from_imsi);
    return nullptr;
}

// get config reference in global variable
spgwConfigStore* spgwConfig::get_cp_config_cpp()
{
    return sub_classifier_config;
}

apn_profile_t* 
spgwConfig::match_apn_profile_cpp(const char *name, uint16_t len)
{
    std::unique_lock<std::mutex> lock(config_mtx);
    std::list<apn_profile_t*>::iterator it;
    LOG_MSG(LOG_DEBUG,"Search APN [%s] in APN profile list", name);
    for (it= sub_classifier_config->apn_profile_list.begin(); it != sub_classifier_config->apn_profile_list.end(); ++it)
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

// return lit of user plane profiles 
int
spgwConfig::get_user_plane_services(user_plane_service_names_t *ptr, int max)
{
    int count=0;
    std::unique_lock<std::mutex> lock(config_mtx);
    for (std::list<user_plane_profile_t*>::iterator it=sub_classifier_config->user_plane_list.begin(); max && it!=sub_classifier_config->user_plane_list.end(); ++it)
    {
        user_plane_profile_t *up=*it;
        strcpy(ptr->user_plane_service , up->user_plane_service);
        ptr->global_address = up->global_address;
        ptr = ptr + 1;
        max--;
        count++;
    }
    return count;
}

int
spgwConfig::parse_cp_json_cpp(cp_config_t *cfg, const char *jsonFile)
{
    LOG_MSG(LOG_INIT,"parsing config in %s ", jsonFile);
    FILE* fp = fopen(jsonFile , "r");
    if(fp == NULL){
        LOG_MSG(LOG_ERROR, "The json config file specified does not exists %s", jsonFile);
        return -1;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    fclose(fp);
    rapidjson::Document doc;
    doc.ParseStream(is);
    if(!doc.IsObject()) {
        LOG_MSG(LOG_ERROR,"Error parsing the json config file %s", jsonFile); 
        return -1;
    }
    if(doc.HasMember("global")) {
        LOG_MSG(LOG_INIT, "global config present");
        const rapidjson::Value& global = doc["global"];
        if(global.HasMember("gxConfig")) {
            cfg->gx_enabled = global["gxConfig"].GetInt();
            LOG_MSG(LOG_INIT,"gx_enabled %d ", cfg->gx_enabled);
        }
        if(global.HasMember("loggingLevel")) {
            std::string log_level = global["loggingLevel"].GetString();
            LOG_MSG(LOG_INIT, "logging level set to %s ",log_level.c_str());
            set_logging_level(log_level.c_str());
        }
        if(global.HasMember("heartbeatFailure")) {
            bool heartbeat_fail = global["heartbeatFailure"].GetBool();
            LOG_MSG(LOG_INIT, "heartbeat failure set to %d ",heartbeat_fail);
            cfg->pfcp_hb_ts_fail = heartbeat_fail;
        } else {
            cfg->pfcp_hb_ts_fail = true;
        }
        if(global.HasMember("periodicTimerSec")) {
            cfg->periodic_timer = global["periodicTimerSec"].GetInt();
            LOG_MSG(LOG_INIT, "periodic_timer = %d ",cfg->periodic_timer);
        }
        if(global.HasMember("requestTimeoutMilliSec")) {
            cfg->request_timeout = global["requestTimeoutMilliSec"].GetInt();
            if(cfg->request_timeout == 5000) {
                cfg->request_timeout = 1500;
            }
            LOG_MSG(LOG_INIT, "request_timeout %d ",cfg->request_timeout);
        }
        if(global.HasMember("requestTries")) {
            cfg->request_tries = global["requestTries"].GetInt();
            LOG_MSG(LOG_INIT,"request_tries = %d ", cfg->request_tries);
        }
        if(global.HasMember("transmitCount")) {
            cfg->transmit_cnt = global["transmitCount"].GetInt();
            LOG_MSG(LOG_INIT, "transmit_cnt = %d ",cfg->transmit_cnt);
        }
        if(global.HasMember("transmitTimerSec")) {
            cfg->transmit_timer = global["transmitTimerSec"].GetInt();
            LOG_MSG(LOG_INIT, "transmit_timer = %d ",cfg->transmit_timer);
        }
        if(global.HasMember("urrConfig")) {
            cfg->urr_enable = global["urrConfig"].GetInt();
            LOG_MSG(LOG_INIT, "urr_enable = %d ", cfg->urr_enable);
        }
        if(global.HasMember("s5s8Port")) {
            cfg->s5s8_port = global["s5s8Port"].GetInt();
            LOG_MSG(LOG_INIT, "s5s8Port = %d ", cfg->s5s8_port);
        } else {
            cfg->s5s8_port = GTPC_UDP_PORT;
        }

        if(global.HasMember("s11Port")) {
            cfg->s11_port = global["s11Port"].GetInt();
            LOG_MSG(LOG_INIT, "s11Port = %d ", cfg->s11_port);
        } else {
            cfg->s11_port = GTPC_UDP_PORT;
        }

        if(global.HasMember("pfcpPort")) {
            cfg->pfcp_port = global["pfcpPort"].GetInt();
            LOG_MSG(LOG_INIT, "pfcpPort = %d ", cfg->pfcp_port);
        } else {
            cfg->pfcp_port = SAEGWU_PFCP_PORT;
        }
        if(global.HasMember("prometheusPort")) {
            cfg->prom_port = global["prometheusPort"].GetInt();
            LOG_MSG(LOG_INIT, "prometheusPort = %d ", cfg->prom_port);
        }
        if(global.HasMember("httpPort")) {
            cfg->webserver_port = global["httpPort"].GetInt();
            LOG_MSG(LOG_INIT, "webserver_port = %d ", cfg->webserver_port);
        } else {
            cfg->webserver_port = HTTP_SERVER_PORT;
        }
        if(global.HasMember("upfdnstimeout")) {
            cfg->upfdnstimeout = global["upfdnstimeout"].GetInt();
            LOG_MSG(LOG_INIT, "upfdnstimeout = %d ", cfg->upfdnstimeout);
        } else {
            cfg->upfdnstimeout = 100;
        }
    }

    if(doc.HasMember("ip_pool_config")) {
        LOG_MSG(LOG_INIT, "ip_pool_config present");
        const rapidjson::Value& ip_pool = doc["ip_pool_config"];
        if(ip_pool.HasMember("staticUeIpPool")) {
            // fixed default static pool
            const rapidjson::Value& ue_pool = ip_pool["staticUeIpPool"];
            std::string pool_name("staticUeIpPool");

            std::string ip = ue_pool["ip"].GetString();
            struct in_addr static_ip_pool_ip={0};
            inet_aton(ip.c_str(), &(static_ip_pool_ip));
            LOG_MSG(LOG_INIT,"static pool network IP %s",inet_ntoa(static_ip_pool_ip));
            static_ip_pool_ip.s_addr = ntohl(static_ip_pool_ip.s_addr);

            struct in_addr static_ip_pool_mask={0};
            std::string mask = ue_pool["mask"].GetString();
            inet_aton(mask.c_str(), &(static_ip_pool_mask));
            LOG_MSG(LOG_INIT,"static pool mask %s",inet_ntoa(static_ip_pool_mask));
            static_ip_pool_mask.s_addr = ntohl(static_ip_pool_mask.s_addr);

            if(cfg->static_ip_pool_ip.s_addr == 0) {
                cfg->static_ip_pool_ip = static_ip_pool_ip;
                cfg->static_ip_pool_mask = static_ip_pool_mask;
                ue_static_pool *pool = new ue_static_pool(pool_name);
                ip_pools::getInstance()->addIpPool(pool, pool_name); 
                pool->gen_pool(cfg->static_ip_pool_ip, cfg->static_ip_pool_mask);
            } else {
                LOG_MSG(LOG_ERROR, "Static Pool can not be changed %s",ip.c_str());
            }
        }
        if(ip_pool.HasMember("ueIpPool")) {
            // fixed default dynamic pool
            const rapidjson::Value& ue_pool = ip_pool["ueIpPool"];
            std::string pool_name("ueIpPool");
            ue_dynamic_pool *pool = new ue_dynamic_pool(pool_name);

            struct in_addr ip_pool_ip{0};
            std::string ip = ue_pool["ip"].GetString();
            inet_aton(ip.c_str(), &(ip_pool_ip));
            LOG_MSG(LOG_INIT,"dynamic pool network %s ",inet_ntoa(ip_pool_ip));
            ip_pool_ip.s_addr = ntohl(ip_pool_ip.s_addr);

            struct in_addr ip_pool_mask={0};
            std::string mask = ue_pool["mask"].GetString();
            inet_aton(mask.c_str(), &(ip_pool_mask));
            LOG_MSG(LOG_INIT, "dynamic Pool mask %s",inet_ntoa(ip_pool_mask));
            ip_pool_mask.s_addr = ntohl(ip_pool_mask.s_addr);
            if(cfg->ip_pool_ip.s_addr == 0) {
                cfg->ip_pool_ip = ip_pool_ip;
                cfg->ip_pool_mask = ip_pool_mask;
                ip_pools::getInstance()->addIpPool(pool, pool_name); 
                pool->gen_pool(cfg->ip_pool_ip, cfg->ip_pool_mask);
            } else {
                LOG_MSG(LOG_ERROR, "Dynamic Pool can not be changed %s",ip.c_str());
            }
        }
        // we may have other pool as well.. 
    }

    return 0;
}
