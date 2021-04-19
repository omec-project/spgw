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
#include <mutex>
#include "ip_pool_mgmt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static struct in_addr native_linux_name_resolve(const char *name);

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
                struct in_addr ip = native_linux_name_resolve(temp); 
                user_plane->upf_addr = ip.s_addr;
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
    sub_classifier_config = config_store; // move to new config
    delete temp;
    return temp_config;
}

sub_profile_t* 
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

    if(it != sub_classifier_config->sub_sel_rules.end())
    {
        // We reached here means we have matching rule 
        sub_profile_t *temp = new (sub_profile_t);
        temp->apn_profile = sub_classifier_config->get_apn_profile(rule->selected_apn_profile);
        temp->qos_profile = sub_classifier_config->get_qos_profile(rule->selected_qos_profile);
        temp->up_profile =  sub_classifier_config->get_user_plane_profile(rule->selected_user_plane_profile);
        temp->access_profile = sub_classifier_config->get_access_profile(rule->selected_access_profile[0]);
        LOG_MSG(LOG_DEBUG,"matching subscriber rule found ");
        return temp;
    }
    LOG_MSG(LOG_INIT,"No matching rule found ");
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

user_plane_profile_t*
spgwConfig::get_user_plane_profile_ref(const char *name)
{
    return sub_classifier_config->get_user_plane_profile(name);
}

void 
spgwConfig::invalidate_user_plane_address(uint32_t addr) 
{
    std::unique_lock<std::mutex> lock(config_mtx);
    for (std::list<user_plane_profile_t*>::iterator it=sub_classifier_config->user_plane_list.begin(); it!=sub_classifier_config->user_plane_list.end(); ++it)
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

// return lit of user plane profiles 
int
spgwConfig::get_user_plane_profiles(profile_names_t *ptr, int max) 
{
    int count=0;
    std::unique_lock<std::mutex> lock(config_mtx);
    for (std::list<user_plane_profile_t*>::iterator it=sub_classifier_config->user_plane_list.begin(); max && it!=sub_classifier_config->user_plane_list.end(); ++it)
    {
        user_plane_profile_t *up=*it;
        strcpy(ptr->profile_name, up->user_plane_profile_name);
        ptr = ptr++;
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

    if(doc.HasMember("dns")) {
        LOG_MSG(LOG_INIT, "dns config present");
        const rapidjson::Value& dns = doc["dns"];
        if(dns.HasMember("app")) {
            LOG_MSG(LOG_INIT, "dns app config present");
            const rapidjson::Value& app = dns["app"];
            if(app.HasMember("frequencySec")) {
              cfg->app_dns.freq_sec = app["frequencySec"].GetInt();
            }
            if(app.HasMember("filename")) {
              std::string temp = app["filename"].GetString();
              strcpy(cfg->app_dns.filename, temp.c_str());
            }
            if(app.HasMember("nameserver")) {
              std::string temp = app["nameserver"].GetString();
              strcpy(cfg->app_dns.nameserver_ip[0], temp.c_str());
            }
        }
        if(dns.HasMember("cache")) {
            LOG_MSG(LOG_INIT, "dns cache config present");
            const rapidjson::Value& cache = dns["cache"];
            if(cache.HasMember("concurrent")) {
              cfg->dns_cache.concurrent = cache["concurrent"].GetInt();
            }
            if(cache.HasMember("intervalSec")) {
              cfg->dns_cache.sec = cache["intervalSec"].GetInt();
            }
            if(cache.HasMember("percentage")) {
              cfg->dns_cache.percent = cache["percentage"].GetInt();
            }
            if(cache.HasMember("queryTimeoutMilliSec")) {
              cfg->dns_cache.timeoutms= cache["queryTimeoutMilliSec"].GetInt();
            }
            if(cache.HasMember("queryTries")) {
              cfg->dns_cache.tries = cache["queryTries"].GetInt();
            }
        }
        if(dns.HasMember("ops")) {
            LOG_MSG(LOG_INIT, "dns ops config present");
            const rapidjson::Value& ops = dns["ops"];
            if(ops.HasMember("filename")) {
              std::string temp = ops["filename"].GetString();
              strcpy(cfg->ops_dns.filename, temp.c_str());
            }
            if(ops.HasMember("nameserver")) {
              std::string temp = ops["nameserver"].GetString();
              strcpy(cfg->ops_dns.nameserver_ip[0], temp.c_str());
            }
            if(ops.HasMember("frequencySec")) {
              cfg->ops_dns.freq_sec = ops["frequencySec"].GetInt();
            }
        }
    }

    return 0;
}

/* Requirement: 
 * For now I am using linux system call to do the service name dns resolution...
 * 3gpp based DNS lookup of NRF support would be required to locate UPF. 
 */
static struct in_addr 
native_linux_name_resolve(const char *name)
{
    struct in_addr ip = {0};
    LOG_MSG(LOG_INFO, "DNS Query - %s ",name);
    struct addrinfo hints;
    struct addrinfo *result=NULL, *rp=NULL;
    int err;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    err = getaddrinfo(name, NULL, &hints, &result);
    if (err == 0)
    {
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            if(rp->ai_family == AF_INET)
            {
                struct sockaddr_in *addrV4 = (struct sockaddr_in *)rp->ai_addr;
                LOG_MSG(LOG_DEBUG, "Received DNS response. name %s mapped to  %s", name, inet_ntoa(addrV4->sin_addr));
                return addrV4->sin_addr;
            }
        }
    }
    LOG_MSG(LOG_ERROR, "DNS Query for %s failed with error %s", name, gai_strerror(err));
    return ip;
}

