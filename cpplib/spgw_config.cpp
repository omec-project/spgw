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
    std::cout<<"parsing config in class function "<<std::endl;
    FILE* fp = fopen(jsonFile , "r");
    if(fp == NULL){
        std::cout << "The json config file specified does not exists" << std::endl;
        return nullptr;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if(!doc.IsObject()) {
        std::cout << "Error parsing the json config file" << std::endl;
        return nullptr;
    }
    spgw_config_profile_t *temp_config = new (spgw_config_profile_t);
    spgwConfigStore *config_store = new (spgwConfigStore);
    temp_config->config = (void *)config_store;

    if(doc.HasMember("subscriber-selection-rules"))
    {
        std::cout << "Subscriber selection rules Array ? "<<doc["subscriber-selection-rules"].IsArray() << std::endl;
        for(uint32_t i=0; i< doc["subscriber-selection-rules"].Size();i++) 
        {
            sub_selection_rule_t *sub_rule = new (sub_selection_rule_t);
            std::cout<<"\tSubscriber selection rule "<<i<<std::endl;
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
                std::cout<<"\t\tkeys has priority "<<sub_rule->rule_priority<<std::endl;
            }
            if(subRuleSection.HasMember("keys"))
            {
                sub_selection_keys_t *key = new (sub_selection_keys_t); 
                key->imsi.is_valid = false;
                key->plmn.is_valid = false;
                key->apn.is_valid = false;
                sub_rule->keys = key;
                const rapidjson::Value& ruleKeys = subRuleSection["keys"];
                std::cout<<"\t\tSubscriber selection rule has keys "<<key<<std::endl;
                if(ruleKeys.HasMember("imsi-range") && ruleKeys["imsi-range"].IsObject())
                {
                    std::cout<<"\t\t\tkeys has imsi-range Object "<<std::endl;
                    const rapidjson::Value& imsiKeys = ruleKeys["imsi-range"];
                    if(imsiKeys.HasMember("from"))
                    {
                        std::cout<<"\t\t\t\tIMSI range from present "<<imsiKeys["from"].GetInt64()<<std::endl;
                    }
                    if(imsiKeys.HasMember("to"))
                    {
                        std::cout<<"\t\t\t\tIMSI range to present "<<imsiKeys["to"].GetInt64()<<std::endl;
                    }
                    key->imsi.is_valid = true;
                    key->imsi.from_imsi= imsiKeys["from"].GetInt64(); 
                    key->imsi.to_imsi= imsiKeys["to"].GetInt64(); 
                }
                if(ruleKeys.HasMember("serving-plmn") && ruleKeys["serving-plmn"].IsObject())
                {
                    std::cout<<"\t\t\tkeys has serving-plmn Object "<<std::endl;
                    const rapidjson::Value& plmnKeys = ruleKeys["serving-plmn"];
                    uint16_t mcc, mnc;
                    if(plmnKeys.HasMember("mcc"))
                    {
                        mcc = plmnKeys["mcc"].GetInt();
                        std::cout<<"\t\t\t\tmcc "<<mcc<<std::endl;
                    }
                    if(plmnKeys.HasMember("mnc"))
                    {
                        mnc = plmnKeys["mnc"].GetInt();
                        std::cout<<"\t\t\t\tmnc "<<mnc<<std::endl;
                    }
                    if(plmnKeys.HasMember("tac"))
                    {
                        std::cout<<"\t\t\t\ttac "<<plmnKeys["tac"].GetInt()<<std::endl;
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

                    std::cout<<"MCC : "<<mcc_dig_1<<" "<<mcc_dig_2<<" "<<mcc_dig_3<<std::endl;
                    std::cout<<"MNC : "<<mnc_dig_1<<" "<<mnc_dig_2<<" "<<mnc_dig_3<<std::endl;
                    key->plmn.plmn[0] = (mcc_dig_2 << 4) | (mcc_dig_1); 
                    key->plmn.plmn[1] = (mnc_dig_1 << 4) | (mcc_dig_3);
                    key->plmn.plmn[2] = (mnc_dig_3 << 4) | (mnc_dig_2);
                    key->plmn.tac = plmnKeys["tac"].GetInt();
                }
                if(ruleKeys.HasMember("requested-apn"))
                {
                    const char *temp = ruleKeys["requested-apn"].GetString();
                    std::cout<<"\t\t\tkeys has requested-apn "<<temp<<std::endl;
                    key->apn.is_valid = true;
                    strcpy(key->apn.requested_apn,temp);
                    
                }
                std::cout<<"Sub key imsi "<<key->imsi.is_valid<<std::endl;
                std::cout<<"Sub key plmn "<<key->plmn.is_valid<<std::endl;
                std::cout<<"Sub key apn "<<key->apn.is_valid<<std::endl;
            }
            std::cout<<"rule->keys "<<sub_rule->keys<<std::endl;
            std::cout<<"\t\tSelected Profiles"<<std::endl;
            if(subRuleSection.HasMember("selected-apn-profile"))
            {
                const char *temp = subRuleSection["selected-apn-profile"].GetString();
                sub_rule->selected_apn_profile = (char *)malloc (strlen(temp)+1);
                std::cout<<"\t\t\tselected-apn-profile found - "<<temp<<std::endl;
                strcpy(sub_rule->selected_apn_profile, temp);
            }
            if(subRuleSection.HasMember("selected-user-plane-profile"))
            {
                const char *temp = subRuleSection["selected-user-plane-profile"].GetString();
                sub_rule->selected_user_plane_profile = (char *)malloc(strlen(temp)+1);
                std::cout<<"\t\t\tselected-user-plane-profile found "<<temp<<std::endl;
                strcpy(sub_rule->selected_user_plane_profile, temp);
            }
            if(subRuleSection.HasMember("selected-qos-profile"))
            {
                const char *temp = subRuleSection["selected-qos-profile"].GetString();
                sub_rule->selected_qos_profile = (char *)malloc(strlen(temp)+1);
                std::cout<<"\t\t\tselected-qos-profile found - "<<temp<<std::endl;
                strcpy(sub_rule->selected_qos_profile, temp);
            }
            if(subRuleSection.HasMember("selected-access-profile"))
            {
                for(uint32_t acc = 0; acc<subRuleSection["selected-access-profile"].Size() && (acc < 4) ; acc++)
                {
                    const rapidjson::Value& accessProfile= subRuleSection["selected-access-profile"][acc];
                    const char *temp = accessProfile.GetString(); 
                    sub_rule->selected_access_profile[acc] = (char *)malloc(strlen(temp)+1);
                    std::cout<<"\t\t\t\tselected-access-profile - "<<temp<<std::endl;
                    strcpy(sub_rule->selected_access_profile[acc], temp);
                }
            }            
            config_store->sub_sel_rules.push_back(sub_rule);
            std::cout<<"Number of selection rules "<<config_store->sub_sel_rules.size()<<std::endl;
            std::cout<<std::endl;
        }
        config_store->sub_sel_rules.sort(compare_sub_rules);
        std::list<sub_selection_rule_t *>::iterator it;
        for (it=config_store->sub_sel_rules.begin(); it!=config_store->sub_sel_rules.end(); ++it)
        {
            sub_selection_rule_t *rule = *it;
            printf("Configured rules with priority  %d \n", rule->rule_priority);
        }
    }
    if(doc.HasMember("apn-profiles"))
    {
        std::cout << "\nAPN profiles is Object ? "<<doc["apn-profiles"].IsObject() << std::endl;
        const rapidjson::Value& apnProfileSection = doc["apn-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = apnProfileSection.MemberBegin(); itr != apnProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            if(itr->value.IsObject())
            {
                std::cout<<"\tAPN profile "<< key.c_str()<<" is Object"<<std::endl; 
                apn_profile_t *apn_profile = new (apn_profile_t);
                memset(apn_profile, 0, sizeof(apn_profile_t));
                strcpy(apn_profile->apn_profile_name, key.c_str());
                const rapidjson::Value& apnSection = itr->value; 
                if(apnSection.HasMember("apn-name")) {
                    const char *temp = apnSection["apn-name"].GetString();
                    std::cout<<"\t\tAPN name["<<temp<<"]"<<std::endl;
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
                    std::cout<<"\t\tAPN name after encode ["<<apn_profile->apn_name<<"]"<<std::endl;
                }
                if(apnSection.HasMember("usage")) {
                    uint16_t usage = apnSection["usage"].GetInt();
                    std::cout<<"\t\tUsage type  "<<usage<<std::endl;
                    apn_profile->apn_usage_type = usage;
                }
                if(apnSection.HasMember("network")) {
                    const char *temp = apnSection["network"].GetString();
                    std::cout<<"\t\tNetwork type "<<temp<<std::endl;
                    strcpy(apn_profile->apn_network_cap, temp);
                }
                if(apnSection.HasMember("gx_enabled")) {
                    bool gx_enabled = apnSection["gx_enabled"].GetBool();
                    std::cout<<"\t\tGx enabled "<<gx_enabled<<std::endl;
                    apn_profile->gx_enabled = gx_enabled;
                }
                if(apnSection.HasMember("mtu")) {
                    uint16_t mtu = apnSection["mtu"].GetInt();
                    std::cout<<"\t\tAPN has mtu "<<mtu<<std::endl;
                    apn_profile->mtu = mtu;
                }
                if(apnSection.HasMember("dns_primary")) {
                    const char *temp = apnSection["dns_primary"].GetString();
                    std::cout<<"\t\tAPN has dns_primary "<<temp<<std::endl;
                    struct in_addr temp_i;
                    inet_aton(temp, &temp_i);
                    apn_profile->dns_primary = temp_i.s_addr;
                }
                if(apnSection.HasMember("dns_secondary")) {
                    const char *temp = apnSection["dns_secondary"].GetString();
                    std::cout<<"\t\tAPN has dns_secondary "<<temp<<std::endl;
                    struct in_addr temp_i;
                    inet_aton(temp, &temp_i);
                    apn_profile->dns_secondary = temp_i.s_addr;
                }
                config_store->apn_profile_list.push_back(apn_profile);
                std::cout<<"Number of APN profiles "<<config_store->apn_profile_list.size()<<std::endl;
                std::cout<<std::endl;
            }
        }
    }
     
    if(doc.HasMember("user-plane-profiles"))
    {
        const rapidjson::Value& userProfileSection = doc["user-plane-profiles"];
        std::cout << "User Plane profiles is Array ? "<<userProfileSection.IsArray() << std::endl;
        for (rapidjson::Value::ConstMemberIterator itr = userProfileSection.MemberBegin(); itr != userProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            user_plane_profile_t *user_plane = new (user_plane_profile_t);
            user_plane->upf_addr = 0;
            const rapidjson::Value& userPlaneSection = itr->value; 
            strcpy(user_plane->user_plane_profile_name, key.c_str());
            if(userPlaneSection.HasMember("user-plane"))
            {
                const char *temp = userPlaneSection["user-plane"].GetString();
                std::cout<<"\tUser Plane - "<<temp<<std::endl;
                strcpy(user_plane->user_plane_service, temp);
            }
            if(userPlaneSection.HasMember("qos-tags"))
            {
                std::cout<<"\t\tQoS Tags specified "<<std::endl;
            }
            if(userPlaneSection.HasMember("access-tags"))
            {
                std::cout<<"\t\tAccess Tags specified "<<std::endl;
            }
            config_store->user_plane_list.push_back(user_plane);
            std::cout<<"Number of User Planes "<<config_store->user_plane_list.size()<<std::endl;
        }
    }
    if(doc.HasMember("qos-profiles"))
    {
        std::cout<<"QoS profiles"<<std::endl;
        const rapidjson::Value& qosProfileSection = doc["qos-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = qosProfileSection.MemberBegin(); itr != qosProfileSection.MemberEnd(); ++itr)
        {
            qos_profile_t *qos_profile = new (qos_profile_t);
            std::string key = itr->name.GetString();
            strcpy(qos_profile->qos_profile_name, key.c_str());
            std::cout<<"\tQoS profile - "<<key<<std::endl;
            const rapidjson::Value& qosPlaneSection = itr->value; 
            qos_profile->apn_ambr_ul = qosPlaneSection["apn-ambr"][0].GetInt64();
            qos_profile->apn_ambr_dl = qosPlaneSection["apn-ambr"][1].GetInt64();
            std::cout<<"\t\tQoS apn ambr uplink - "<<qos_profile->apn_ambr_ul <<" downlink - "<<qos_profile->apn_ambr_dl<<std::endl;
            config_store->qos_profile_list.push_back(qos_profile);
            std::cout<<"Number of QoS profiles "<<config_store->qos_profile_list.size()<<std::endl;
        }
    }
    if(doc.HasMember("access-profiles"))
    {
        std::cout<<"Access profiles"<<std::endl;
        const rapidjson::Value& accessProfileSection = doc["access-profiles"];
        for (rapidjson::Value::ConstMemberIterator itr = accessProfileSection.MemberBegin(); itr != accessProfileSection.MemberEnd(); ++itr)
        {
            std::string key = itr->name.GetString();
            std::cout<<"\tAccess profile - "<<key<<std::endl;
            access_profile_t *access_profile = new (access_profile_t);
            strcpy(access_profile->access_profile_name, key.c_str());
            config_store->access_profile_list.push_back(access_profile);
            std::cout<<"Number of Access profiles "<<config_store->access_profile_list.size()<<std::endl;
        }
    }    
    return temp_config;
}

sub_profile_t* 
spgwConfig::match_sub_selection_cpp(sub_selection_keys_t *key)
{
    sub_selection_rule_t *rule = nullptr;
    std::list<sub_selection_rule_t *>::iterator it;
    std::cout<<"IMSI - "<<key->imsi.from_imsi<<std::endl;
    for (it=config->sub_sel_rules.begin(); it!=config->sub_sel_rules.end(); ++it)
    {
        rule = *it;
        printf("Searching rule %d \n", rule->rule_priority);
        sub_selection_keys_t *key_l = rule->keys;
        std::cout<<"rule->key imsi valid : "<<key_l->imsi.is_valid<<" search key imsi valid "<<key->imsi.is_valid<<std::endl;
        if((key_l != nullptr) && (key_l->imsi.is_valid))
        {
            if(key->imsi.is_valid == false)
            {
                continue; // no match continue for next rule 
            }
            std::cout<<"search key from imsi "<<key->imsi.from_imsi<<" to_imsi "<<key->imsi.to_imsi<<std::endl;
            std::cout<<"rule->key from imsi "<<key_l->imsi.from_imsi<<" to_imsi "<<key_l->imsi.to_imsi<<std::endl;
            if(!((key->imsi.from_imsi >= key_l->imsi.from_imsi) && (key->imsi.from_imsi <= key_l->imsi.to_imsi)))
               continue; // no match continue for next rule  
        }
        printf("IMSI matched \n");
        printf("key_l = %p ", key_l);
        if(key_l)
            std::cout<<"key_l->plmn "<<key_l->plmn.is_valid<<std::endl;
        if((key_l != nullptr) && (key_l->plmn.is_valid))
        {
            if(key->plmn.is_valid == false)
                continue; // no match . Continue for next rule 
            printf("plmn valid \n");
            if(memcmp(&key_l->plmn.plmn[0], &key_l->plmn.plmn[0],3))
                continue; // no match 
            if(key_l->plmn.tac != key->plmn.tac)
                continue; // no match 
        } 
        printf("PLMN matched \n");
        if((key_l != nullptr) && (key_l->apn.is_valid))
        {
            if(key->apn.is_valid == false)
                continue;
            printf("apn valid \n");
            if(strcmp(key->apn.requested_apn, key_l->apn.requested_apn))
                continue;
        }
        printf("APN matched \n");
        printf("Profile found \n");
        break;
    }
    std::cout<<"Rule Search finished"<<std::endl;

    if(it != config->sub_sel_rules.end())
    {
        // We reached here means we have matching rule 
        sub_profile_t *temp = new (sub_profile_t);
        temp->apn_profile = config->get_apn_profile(rule->selected_apn_profile);
        temp->qos_profile = config->get_qos_profile(rule->selected_qos_profile);
        temp->up_profile = config->get_user_plane_profile(rule->selected_user_plane_profile);
        temp->access_profile = config->get_access_profile(rule->selected_access_profile[0]);
        return temp;
    }
    return nullptr;
}

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
    for (it=config->apn_profile_list.begin(); it!=config->apn_profile_list.end(); ++it)
    {
        apn_profile_t* apn = *it;
        printf("Compare APN profile [%s] with [%s] \n", apn->apn_name, name);
        if(strlen(apn->apn_name) != len)
            continue;
        if(memcmp(apn->apn_name,name,len) != 0)
            continue;
        printf("Found APN profile   %s \n", apn->apn_name);
        return apn;
    }
    printf("APN [%s] not found \n", name);
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
            std::cout<<"invalidating upf address. \n";
            up->upf_addr = 0;
        }
    }
    return ;
}

