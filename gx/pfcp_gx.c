// Copyright 2020-present Open Networking Foundation
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include "pfcp.h"
#include "cp_config.h"
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_cfgfile.h>
#include <rte_string_fns.h>

#include "gw_adapter.h"
#include "clogger.h"
#include "sm_structs_api.h"
#include "spgw_cpp_wrapper.h"
//REVIEW: Need to check this: No need to add this header files
#include "pfcp.h"
#include "sm_enum.h"
#include "pfcp_cp_util.h"
#include "cp_config.h"
#include "pfcp_cp_session.h"
#include "pfcp_messages.h"
#include "pfcp_messages_encoder.h"
#include "gtpv2_set_ie.h"
#include "gtpv2_interface.h"
#include "gen_utils.h"
#include "cp_transactions.h"
#include "tables/tables.h"
#include "util.h"
#include "cp_io_poll.h"
#include "proc_rar.h"

#define PRESENT 1
#define NUM_VALS 9

/* Default Bearer Indication Values */
#define BIND_TO_DEFAULT_BEARER			0
#define BIND_TO_APPLICABLE_BEARER		1
/* Default Bearer Indication Values */

#define SET_EVENT(val,event) (val |=  (1<<event))

extern uint8_t gtp_tx_buf[MAX_GTPV2C_UDP_LEN];

/**
 * @brief  : Fill UE context default bearer information of default_eps_bearer_qos from CCA
 * @param  : context , eps bearer context
 * @param  : cca
 * @return : Returns 0 in case of success , -1 otherwise
 * */
static int
store_default_bearer_qos_in_policy(pdn_connection_t *pdn, GxDefaultEpsBearerQos qos)
{
	int8_t ebi_index = 0;
	eps_bearer_t *bearer = NULL;
	if (pdn == NULL)
		return -1;
	ebi_index = pdn->default_bearer_id - 5;
	bearer = pdn->eps_bearers[ebi_index];


	pdn->policy.default_bearer_qos_valid = TRUE;
	bearer_qos_ie *def_qos = &pdn->policy.default_bearer_qos;


	if (qos.presence.qos_class_identifier == PRESENT) {
		def_qos->qci = qos.qos_class_identifier;
		bearer->qos.qci = qos.qos_class_identifier;
	}

    // GXFIX - 
	if(qos.presence.allocation_retention_priority == PRESENT) {
		if(qos.allocation_retention_priority.presence.priority_level == PRESENT){
			def_qos->arp.priority_level = qos.allocation_retention_priority.priority_level;
			bearer->qos.arp.priority_level = qos.allocation_retention_priority.priority_level;
		}
		if(qos.allocation_retention_priority.presence.pre_emption_capability == PRESENT){
			def_qos->arp.preemption_capability = qos.allocation_retention_priority.pre_emption_capability;
			bearer->qos.arp.preemption_capability = qos.allocation_retention_priority.pre_emption_capability;
		}
		if(qos.allocation_retention_priority.presence.pre_emption_vulnerability == PRESENT){
			def_qos->arp.preemption_vulnerability = qos.allocation_retention_priority.pre_emption_vulnerability;
			bearer->qos.arp.preemption_vulnerability = qos.allocation_retention_priority.pre_emption_vulnerability;
		}
	}

	return 0;
}

#if 0
/**
 * @brief  : Fill qos information for dedicated bearer form charging rules
 * @param  : bearer , eps bearer to be modified
 * @param  : rule_defination , charging rule details
 * @return : Returns nothing
 */
static void
fill_dedicated_bearer_qos(eps_bearer_t *bearer, GxChargingRuleDefinition *rule_definition)
{
	GxQosInformation *qos = &(rule_definition->qos_information);

	bearer->qos.qci = qos->qos_class_identifier;
	bearer->qos.arp.priority_level = qos->allocation_retention_priority.priority_level;
	bearer->qos.arp.preemption_capability = qos->allocation_retention_priority.pre_emption_capability;
	bearer->qos.arp.preemption_vulnerability = qos->allocation_retention_priority.pre_emption_vulnerability;
	bearer->qos.ul_mbr =  qos->max_requested_bandwidth_ul;
	bearer->qos.dl_mbr =  qos->max_requested_bandwidth_dl;
	bearer->qos.ul_gbr =  qos->guaranteed_bitrate_ul;
	//bearer->qos.ul_gbr =  qos->guaranteed_requested_bandwidth_ul;
	bearer->qos.dl_gbr =  qos->guaranteed_bitrate_dl;
	//bearer->qos.dl_gbr =  qos->guaranteed_requested_bandwidth_dl;
}

/**
 * @brief  : Extracts charging rule name from given data
 * @param  : rule_definition , charging rule details
 * @param  : rule_name , variable to store extracted rule name
 * @return : returns nothing
 */
static void
get_charging_rule_name(GxChargingRuleDefinition *rule_definition, char rule_name[])
{
	rule_name[0] = '\0';
	if (rule_definition->presence.charging_rule_name == PRESENT)
	{
		memcpy(rule_name,
				rule_definition->charging_rule_name.val,
				rule_definition->charging_rule_name.len);

		rule_name[rule_definition->charging_rule_name.len] = '\0';
	}
}
#endif

/**
 * @brief  : Extracts  data form sdf string str and fills into packet filter
 * @param  : str
 * @param  : pkt_filter
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
fill_sdf_strctr(char *temp_str, sdf_pkt_fltr *pkt_filter, pdn_connection_t *pdn)
{
	int nb_token = 0;
	char *str_fld[NUM_VALS];
	int offset = 0;
    printf("string to parse = %s \n", temp_str);
    char str[512];
    strcpy(str, temp_str);

	/* VG: format of sdf string is  */
	/* action dir fom src_ip src_port to dst_ip dst_port" */
    /* permit out 17 from 172.30.0.109 49090 to 100.65.0.3 50024 */
    /* 0       1   2  3        4         5   6   7          8*/
    //   permit out udp from any to assigned
    //    0      1   2   3    4   5  6
	nb_token = rte_strsplit(str, strlen(str), str_fld, NUM_VALS, ' ');
    printf("nb_token = %d \n", nb_token);
	if (nb_token > NUM_VALS) {
		clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Reach Max limit for sdf string \n",
				__file__, __func__, __LINE__);
        printf("%s %d \n",__FUNCTION__, __LINE__);
		return -1;
	}

	for(int indx=0; indx < nb_token; indx++) {
        printf("\n Index %d string %s \n", indx, str_fld[indx]);
        if( indx == 0 ) {
            if(strncmp(str_fld[indx], "permit", strlen("permit")) != 0 ) {
                clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Skip sdf filling for IP filter rule action : %s \n",
                        __file__, __func__, __LINE__,str_fld[indx]);
                printf("%s %d \n",__FUNCTION__, __LINE__);
                return -1;
            } else {
                printf("permit \n");
            }
		} else if(indx == 2) {
			pkt_filter->proto_id = atoi(str_fld[indx]);
            printf("\n proto_id %d \n",pkt_filter->proto_id);
		} else if (indx == 4) {
			if(strncmp(str_fld[indx], "any", strlen("any")) != 0) {
        printf("any not present in the filter \n");
				if( strstr(str_fld[indx], "/") != NULL) {
                    printf("/ present in the filter \n");
					int ip_token = 0;
					char *ip_fld[2];
					ip_token = rte_strsplit(str_fld[indx], strlen(str_fld[indx]), ip_fld, 2, '/');
					if (ip_token > 2) {
						clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Reach Max limit for sdf src ip \n",
							__file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
						return -1;
					}
					if(inet_pton(AF_INET, (const char *) ip_fld[0], (void *)(&pkt_filter->local_ip_addr)) < 0){
						clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:conv of src ip fails \n",
							__file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
						return -1;
					}

					pkt_filter->local_ip_mask = atoi(ip_fld[1]);
				} else {
					if(inet_pton(AF_INET, (const char *) str_fld[indx], (void *)(&pkt_filter->local_ip_addr)) < 0){
						clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:conv of src ip fails \n",
								__file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
						return -1;
					}
                    printf("/ is not present in the filter %s \n",inet_ntoa(pkt_filter->local_ip_addr));
				}
			}
		} else if(indx == 5) {
			/*TODO VG : handling of multiple ports p1,p2,p3 etc*/
			if(strncmp(str_fld[indx], "to", strlen("to")) != 0) {
        printf("\n no TO found \n");
				if( strstr(str_fld[indx], "-") != NULL) {
                    printf("\n found - \n");
					int port_token = 0;
					char *port_fld[2];
					port_token = rte_strsplit(str_fld[indx], strlen(str_fld[indx]), port_fld, 2, '-');

					if (port_token > 2) {
						clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Reach Max limit for sdf src port \n",
							__file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
						return -1;
					}

					pkt_filter->local_port_low = atoi(port_fld[0]);
					pkt_filter->local_port_high = atoi(port_fld[1]);

				} else {
					pkt_filter->local_port_low = atoi(str_fld[indx]);
					pkt_filter->local_port_high = atoi(str_fld[indx]);
                    printf("\n pkt_filter->local_port_low %d pkt_filter->local_port_high %d \n", pkt_filter->local_port_low, pkt_filter->local_port_high);
				}
			}else {
                printf("\n TO found...");
				offset++;
			}
		} else if (indx + offset == 7) {

			if(strncmp(str_fld[indx], "any", strlen("any")) == 0 ) {
                printf("keyword any used in the ip filter\n");
            }
			else if(strncmp(str_fld[indx], "assigned", strlen("assigned")) == 0 ) {
                printf("keyword assigned used in the ip filter\n");
                pkt_filter->remote_ip_mask = 32;
                pkt_filter->remote_ip_addr = pdn->ipv4;
            }
            else {
                if( strstr(str_fld[indx], "/") != NULL) {
                    int ip_token = 0;
                    char *ip_fld[2];
                    ip_token = rte_strsplit(str_fld[indx], strlen(str_fld[indx]), ip_fld, 2, '/');
                    if (ip_token > 2) {
                        clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Reach Max limit for sdf dst ip \n",
                                __file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
                        return -1;
                    }
                    if(inet_pton(AF_INET, (const char *) ip_fld[0], (void *)(&pkt_filter->remote_ip_addr)) < 0){
                        clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:conv of dst ip fails \n",
                                __file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
                        return -1;
                    }

                    pkt_filter->remote_ip_mask = atoi(ip_fld[1]);
                } else{
                    if(inet_pton(AF_INET, (const char *) str_fld[indx], (void *)(&pkt_filter->remote_ip_addr)) < 0){
                        clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:conv of dst ip \n",
                                __file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
                        return -1;
                    }
                }
            }
		} else if(indx + offset == 8) {
			/*TODO VG : handling of multiple ports p1,p2,p3 etc*/

			if( strstr(str_fld[indx], "-") != NULL) {
				int port_token = 0;
				char *port_fld[2];
				port_token = rte_strsplit(str_fld[indx], strlen(str_fld[indx]), port_fld, 2, '-');

				if (port_token > 2) {
					clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:Reach Max limit for sdf dst port\n",
							__file__, __func__, __LINE__);
                        printf("%s %d \n",__FUNCTION__, __LINE__);
					return -1;
				}

				pkt_filter->remote_port_low = atoi(port_fld[0]);
				pkt_filter->remote_port_high = atoi(port_fld[1]);

			} else {
                printf("pkt_filter->remote_port_low %d pkt_filter->remote_port_high %d \n",pkt_filter->remote_port_low, pkt_filter->remote_port_high);
				pkt_filter->remote_port_low = atoi(str_fld[indx]);
				pkt_filter->remote_port_high = atoi(str_fld[indx]);
			}
		}
	}

    printf("success %s %d \n",__FUNCTION__, __LINE__);
    return 0;
}

/**
 * @brief  : Fills dynamic rule from given charging rule definition , and adds mapping of rule and bearer id
 * @param  : dynamic_rule
 * @param  : rule_definition
 * @param  : bearer_id
 * @return : Returns 0 in case of success , -1 otherwise
 */
// charging rule defination is per dynamic rule 
static int
fill_charging_rule_definition(pdn_connection_t *pdn, 
                              dynamic_rule_t *dynamic_rule,
					          GxChargingRuleDefinition *rule_definition)
{
    int32_t idx = 0;
    printf("%s %d  - dynamic rule %p \n", __FUNCTION__, __LINE__, dynamic_rule);

    /* VS: Allocate memory for dynamic rule */
    //	dynamic_rule = rte_zmalloc_socket(NULL, sizeof(dynamic_rule_t),
    //	    RTE_CACHE_LINE_SIZE, rte_socket_id());
    //	if (dynamic_rule == NULL) {
    //		clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate bearer id memory "
    //				"structure: %s (%s:%d)\n",
    //				rte_strerror(rte_errno),
    //				__file__,
    //				__LINE__);
    //		return -1;
    //	}

    if (rule_definition->presence.online == PRESENT)
        dynamic_rule->online =  rule_definition->online;

    if (rule_definition->presence.offline == PRESENT)
        dynamic_rule->offline = rule_definition->offline;

    if (rule_definition->presence.flow_status == PRESENT)
        dynamic_rule->flow_status = rule_definition->flow_status;

    if (rule_definition->presence.reporting_level == PRESENT)
        dynamic_rule->reporting_level = rule_definition->reporting_level;

    if (rule_definition->presence.precedence == PRESENT)
        dynamic_rule->precedence = rule_definition->precedence;

    if (rule_definition->presence.service_identifier == PRESENT)
        dynamic_rule->service_id = rule_definition->service_identifier;

    if (rule_definition->presence.rating_group == PRESENT)
        dynamic_rule->rating_group = rule_definition->rating_group;

    if (rule_definition->presence.default_bearer_indication == PRESENT)
        dynamic_rule->def_bearer_indication = rule_definition->default_bearer_indication;
    else
        dynamic_rule->def_bearer_indication = BIND_TO_APPLICABLE_BEARER;


    if (rule_definition->presence.af_charging_identifier == PRESENT)
    {
        /* CHAR*/
        memcpy(dynamic_rule->af_charging_id_string,
                rule_definition->af_charging_identifier.val,
                rule_definition->af_charging_identifier.len);
    }

    printf("rule_definition->presence.flow_information = %d \n", rule_definition->presence.flow_information);
    if (rule_definition->presence.flow_information == PRESENT) {
        dynamic_rule->num_flw_desc = rule_definition->flow_information.count;

        for(idx = 0; idx < rule_definition->flow_information.count; idx++)
        {
            printf("\n index %d \n", idx);
            if ((rule_definition->flow_information).list[idx].presence.flow_direction
                    == PRESENT) {
                dynamic_rule->flow_desc[idx].flow_direction =
                    (rule_definition->flow_information).list[idx].flow_direction;
            }

            /* CHAR*/
            if ((rule_definition->flow_information).list[idx].presence.flow_description
                    == PRESENT) {
                memcpy(dynamic_rule->flow_desc[idx].sdf_flow_description,
                        (rule_definition->flow_information).list[idx].flow_description.val,
                        (rule_definition->flow_information).list[idx].flow_description.len);
                dynamic_rule->flow_desc[idx].flow_desc_len =
                    (rule_definition->flow_information).list[idx].flow_description.len;

                printf("calling fill_sdf_strctr - flow description is present %s %d \n",dynamic_rule->flow_desc[idx].sdf_flow_description, dynamic_rule->flow_desc[idx].flow_desc_len);
                fill_sdf_strctr(dynamic_rule->flow_desc[idx].sdf_flow_description,
                                &(dynamic_rule->flow_desc[idx].sdf_flw_desc),
                                pdn);

                printf("after fill_sdf_strctr - flow description is present %s %d \n",dynamic_rule->flow_desc[idx].sdf_flow_description, dynamic_rule->flow_desc[idx].flow_desc_len);
                /*VG assign direction in flow desc */
                dynamic_rule->flow_desc[idx].sdf_flw_desc.direction =(uint8_t)
                    (rule_definition->flow_information).list[idx].flow_direction;

            }
        }
    }


    printf("rule_definition->presence.qos_information %d \n", rule_definition->presence.qos_information);
    if(rule_definition->presence.qos_information == PRESENT) {
        printf("Found qos informaation in the rule \n");
        GxQosInformation *qos = &(rule_definition->qos_information);
        dynamic_rule->qos.qci = qos->qos_class_identifier;
        printf("dynamic_rule->qos.qci %d \n", dynamic_rule->qos.qci);
        dynamic_rule->qos.arp.priority_level = qos->allocation_retention_priority.priority_level;
        dynamic_rule->qos.arp.preemption_capability = qos->allocation_retention_priority.pre_emption_capability;
        dynamic_rule->qos.arp.preemption_vulnerability = qos->allocation_retention_priority.pre_emption_vulnerability;
        dynamic_rule->qos.ul_mbr =  qos->max_requested_bandwidth_ul;
        dynamic_rule->qos.dl_mbr =  qos->max_requested_bandwidth_dl;
        dynamic_rule->qos.ul_gbr =  qos->guaranteed_bitrate_ul;
        dynamic_rule->qos.dl_gbr =  qos->guaranteed_bitrate_dl;
    }

    if (rule_definition->presence.charging_rule_name == PRESENT) {
        rule_name_key_t key = {0};
        /* Commenting for compliation error Need to check
           id.bearer_id = bearer_id; */

        strncpy(key.rule_name, (char *)(rule_definition->charging_rule_name.val),
                rule_definition->charging_rule_name.len);

        memset(dynamic_rule->rule_name, '\0', sizeof(dynamic_rule->rule_name));
        strncpy(dynamic_rule->rule_name,
                (char *)rule_definition->charging_rule_name.val,
                rule_definition->charging_rule_name.len);
        printf("dynamic rule found = %s \n",dynamic_rule->rule_name);
#if 0
        /* VS: Maintain the Rule Name and Bearer ID  mapping with call id */
        if (add_rule_name_entry(key, &id) != 0) {
            clLog(clSystemLog, eCLSeverityCritical, "%s:%d Failed to add_rule_name_entry with rule_name\n",
                    __func__, __LINE__);
            return -1;
        }
#endif
    }

    return 0;
}

/**
 * @brief  : store the event tigger value received in CCA
 * @param  : pdn
 * @param  : GxEventTriggerList
 * @return : Returns 0 in case of success
 */
static int
store_event_trigger(pdn_connection_t *pdn, GxEventTriggerList *event_trigger)
{
	if(event_trigger != NULL) {
		for(uint8_t i = 0; i < event_trigger->count; i++) {
			int32_t val =  *event_trigger->list;

			/*Jumping the list to the next GxEventTriggerList*/
			event_trigger->list++;
			//pdn->context->event_trigger = val;
			//set_event_trigger_bit(pdn->context)
			SET_EVENT(pdn->context->event_trigger, val);
		}
	}

	return 0;
}

/**
 * @brief  : Creates and fills dynamic rules for given bearer from received cca
 * @param  : context , eps bearer context
 * @param  : cca
 * @param  : bearer_id
 * @return : Returns 0 in case of success , -1 otherwise
 */
static int
store_dynamic_rules_in_policy(pdn_connection_t *pdn, GxChargingRuleInstallList * charging_rule_install,
		GxChargingRuleRemoveList * charging_rule_remove, bool install_present, bool remove_present)
{

	int32_t idx = 0;
	rule_name_key_t rule_name = {0};
	GxChargingRuleDefinition *rule_definition = NULL;

  printf("%s pdn->policy.count %d \n",__FUNCTION__,pdn->policy.count);
  printf("%s charging_rule_install->count %d \n",__FUNCTION__,charging_rule_install->count);

	if(install_present)
	{
		for (int32_t idx1 = 0; idx1 < charging_rule_install->count; idx1++, pdn->policy.count++)
		{
            // GXASSUMPTIONS : predefined rules work by using charging_rule_install->name, base_name...
            // We dont support predefined rules as of now 
			if (charging_rule_install->list[idx1].presence.charging_rule_definition == PRESENT)
			{
				for(int32_t idx2 = 0; idx2 < charging_rule_install->list[idx].charging_rule_definition.count; idx2++)
				{
					rule_definition =
						&(charging_rule_install->list[idx].charging_rule_definition.list[idx2]);
					if (rule_definition->presence.charging_rule_name == PRESENT) {
                        pcc_rule_t *pcc_rule = calloc(1, sizeof(pcc_rule_t)); 
                        pcc_rule->dyn_rule = calloc(1, sizeof(dynamic_rule_t)); 
						memset(rule_name.rule_name, '\0', sizeof(rule_name.rule_name));
						strncpy(rule_name.rule_name, (char *)(rule_definition->charging_rule_name.val),
								rule_definition->charging_rule_name.len);
						sprintf(rule_name.rule_name, "%s%d",
								rule_name.rule_name, pdn->call_id);
						if(get_rule_name_entry(rule_name) == -1) {
							pcc_rule->action = RULE_ACTION_ADD;
							pdn->policy.num_charg_rule_install++;
						} else {
							pcc_rule->action =  RULE_ACTION_MODIFY;
							pdn->policy.num_charg_rule_modify++;
						}
                        // GXFIX - looks like we are overriding pcc rules if we have multiple charging rule install
                        // We should implemtnet index..or keep policy as link list in pdn...
                        TAILQ_INSERT_TAIL(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
						fill_charging_rule_definition(pdn, pcc_rule->dyn_rule, rule_definition);
					} else {
						//TODO: Rule without name not possible; Log IT ?
                        printf("Rule without name not allowed\n");
						return -1;
					}
				}
			}
		}
	}

	if(remove_present)
	{
		for(int32_t idx1 = 0; idx1 < charging_rule_remove->count; idx1++,pdn->policy.count++)
		{
			if (charging_rule_remove->list[idx1].presence.charging_rule_name == PRESENT)
			{
				//Get the rule name and only store the name in dynamic rule_t
				memset(rule_name.rule_name, '\0', 255);
				strncpy(rule_name.rule_name,
						(char *)(charging_rule_remove->list[idx1].charging_rule_name.list[0].val),
						charging_rule_remove->list[idx1].charging_rule_name.list[0].len);
				sprintf(rule_name.rule_name, "%s%d",
						rule_name.rule_name, pdn->call_id);
				/* TODO: Need to remove comment */
				int8_t bearer_identifer = get_rule_name_entry(rule_name);
				if (bearer_identifer >= 0)
				{
                    pcc_rule_t *pcc_rule = calloc(1, sizeof(pcc_rule_t)); 
                    pcc_rule->dyn_rule = calloc(1, sizeof(dynamic_rule_t)); 
					pcc_rule->action = RULE_ACTION_DELETE;
					pdn->policy.num_charg_rule_delete++;
					memset(pcc_rule->dyn_rule->rule_name, '\0', 256);
					strncpy(pcc_rule->dyn_rule->rule_name,
							(char *)(charging_rule_remove->list[idx1].charging_rule_name.list[0].val),
							charging_rule_remove->list[idx1].charging_rule_name.list[0].len);
                    // GXBUG : idx_offset is not incremented..it will keep overriding rules 
                    TAILQ_INSERT_TAIL(&pdn->policy.pending_pcc_rules, pcc_rule, next_pcc_rule);
				} else {
                        printf("Rule %s - Received in RAR and not found \n", rule_name.rule_name);
        }
			}
		}
	}

#if 0
	for (idx = 0; idx < cca->charging_rule_install.count; idx++)
	{
	        if ((cca->charging_rule_install).list[idx].presence.charging_rule_definition == PRESENT)
	                for (indx = 0;
							indx < (cca->charging_rule_install).list[idx].charging_rule_definition.count;
							indx++)
	                {
						context->num_dynamic_filters =
						        (cca->charging_rule_install).list[idx].charging_rule_definition.count;

						for (cnt = 0; cnt < context->num_dynamic_filters; cnt++) {
							/* VS: Allocate memory for dynamic rule */
							context->dynamic_rules[cnt] = rte_zmalloc_socket(NULL, sizeof(dynamic_rule_t),
							    RTE_CACHE_LINE_SIZE, rte_socket_id());
							if (context->dynamic_rules[cnt] == NULL) {
								clLog(clSystemLog, eCLSeverityCritical, "Failure to allocate dynamic rule memory "
										"structure: %s (%s:%d)\n",
										rte_strerror(rte_errno),
										__file__,
										__LINE__);
								return -1;
							}

	                        fill_charging_rule_definition(context->dynamic_rules[cnt],
	                                        (GxChargingRuleDefinition *)
											&((cca->charging_rule_install).list[idx].charging_rule_definition.list[cnt]),
											bearer_id);
						}
	                }
	}
#endif
	return 0;

}

static int
check_for_rules_on_default_bearer(pdn_connection_t *pdn)
{
    pcc_rule_t *pcc_rule = TAILQ_FIRST(&pdn->policy.pending_pcc_rules);
    while (pcc_rule != NULL) {
        pcc_rule_t *next_pcc_rule = TAILQ_NEXT(pcc_rule, next_pcc_rule);
		if ((BIND_TO_DEFAULT_BEARER == pcc_rule->dyn_rule->def_bearer_indication) ||
			(compare_default_bearer_qos(&pdn->policy.default_bearer_qos,
					&pcc_rule->dyn_rule->qos) == 0))
        {
            printf("\n check_for_rules_on_default_bearer - installing rules on default bearer \n");
            /* Adding rule and bearer id to a hash */
            bearer_id_t *id;
            id = malloc(sizeof(bearer_id_t));
            memset(id, 0 , sizeof(bearer_id_t));
            rule_name_key_t key = {0};
            id->bearer_id = pdn->default_bearer_id - 5;
            strncpy(key.rule_name, pcc_rule->dyn_rule->rule_name,
                    strlen(pcc_rule->dyn_rule->rule_name));
            sprintf(key.rule_name, "%s%d", key.rule_name,
                    pdn->call_id);
            if (add_rule_name_entry(key, id) != 0) {
                clLog(sxlogger, eCLSeverityCritical,
                        FORMAT"Failed to add_rule_name_entry with rule_name\n",
                        ERR_MSG);
                return -1;
            }
            //GXFEATURE : we can have multiple rules on default bearer...
            return 0;
        }
        pcc_rule = next_pcc_rule;
	}
    printf("%s %d \n", __FUNCTION__, __LINE__);
	return 0;
}

#if 0
/**
 * @brief  : Retrives bearer id which has same qos info as in cca
 * @param  : pdn
 * @param  : cca
 * @return : Returns bearer id in case of success , -1 otherwise
 */
static int8_t
retrieve_bearer_id(pdn_connection_t *pdn, GxCCA *cca)
{
	int32_t idx = 0, indx = 0;
	int8_t ret = 0, id = 0;


	for (idx = 0; idx < cca->charging_rule_install.count; idx++)
	{
	    if ((cca->charging_rule_install).list[idx].presence.charging_rule_definition == PRESENT) {
			for (indx = 0;
				indx < (cca->charging_rule_install).list[idx].charging_rule_definition.count;
				indx++)
			{
				for (id = 0;
						id < MAX_BEARERS;
						id++)
				{
					if (pdn->eps_bearers[id] == NULL)
						continue;

					if ((cca->charging_rule_install).list[idx].charging_rule_definition.list[indx].presence.qos_information == PRESENT) {
						ret = (compare_default_bearer_qos(&(pdn->eps_bearers[id])->qos,
								(GxQosInformation *)
								&((cca->charging_rule_install).list[idx].charging_rule_definition.list[indx].qos_information)));
						if (ret == 0)
							return id;
					} else {
						clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:charging_rule_definition missing:"
								"AVP:Qos Information \n",
								__file__, __func__, __LINE__);
						return -1;
					}
				}
	       }
		}
	}
	return -1;
}
#endif

/* VS: TODO: Parse gx CCA response and fill UE context and pfcp context */
int8_t
parse_gx_cca_msg(GxCCA *cca, pdn_connection_t **_pdn)
{

	int ret = 0;
	uint32_t call_id = 0;
	pdn_connection_t *pdn_cntxt = NULL;

	/* Extract the call id from session id */
	ret = retrieve_call_id((char *)&cca->session_id.val, &call_id);
	if (ret < 0) {
	        clLog(clSystemLog, eCLSeverityCritical, "%s:No Call Id found from session id:%s\n", __func__,
	                        cca->session_id.val);
	        return -1;
	}

	/* Retrieve PDN context based on call id */
	pdn_cntxt = get_pdn_conn_entry(call_id);
	if (pdn_cntxt == NULL)
	{
	      clLog(clSystemLog, eCLSeverityCritical, "%s:No valid pdn cntxt found for CALL_ID:%u\n",
	                          __func__, call_id);
	      return -1;
	}
	*_pdn = pdn_cntxt;

	/* Fill the BCM */
	pdn_cntxt->bearer_control_mode = cca->bearer_control_mode;


	/* VS: Overwirte the CSR qos values with CCA default eps bearer qos values */
	if (cca->presence.default_eps_bearer_qos != PRESENT) {
		clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:default_eps_bearer_qos is missing \n",
				__file__, __func__, __LINE__);
		return -1;
	}
	ret = store_default_bearer_qos_in_policy(pdn_cntxt, cca->default_eps_bearer_qos);
	if (ret) {
        clLog(gxlogger, eCLSeverityCritical, "%s:%s:%d AVP:default_eps_bearer_qos failed to store \n",
                __file__, __func__, __LINE__);
        return ret;
    }


	/* VS: Compare the default qos and CCA charging rule qos info and retrieve the bearer identifier */
	//bearer_id = retrieve_bearer_id(pdn_cntxt, cca);
	//if (bearer_id)
	  //      return bearer_id;


	/* VS: Fill the dynamic rule from rule install structure of cca to policy */
    bool install_rule_present = (cca->presence.charging_rule_install == PRESENT);
    bool remove_rule_present = (cca->presence.charging_rule_remove == PRESENT);
	ret = store_dynamic_rules_in_policy(pdn_cntxt, &(cca->charging_rule_install), &(cca->charging_rule_remove), install_rule_present, remove_rule_present);
    if (ret) {
        printf("Failed at %s %d \n",__FUNCTION__, __LINE__);
        return ret;
    }

	ret = check_for_rules_on_default_bearer(pdn_cntxt);
    if (ret) {
        printf("Failed at %s %d \n",__FUNCTION__, __LINE__);
        return ret;
    }

	if (cca->presence.event_trigger == PRESENT) {
        printf("Trigger present in the message \n");
        ret = store_event_trigger(pdn_cntxt, &(cca->event_trigger));
        if (ret) {
            printf("Failed at %s %d \n",__FUNCTION__, __LINE__);
            return ret;
        }
    }

	return ret;
}

int16_t
gx_update_bearer_req(pdn_connection_t *pdn)
{

	int ret = 0;
	eps_bearer_t *bearer = NULL;
	ue_context_t *context = NULL;
	int update_require = 0, send_ubr = 0;
	uint8_t len = 0;
	uint16_t payload_length = 0;

	bzero(&gtp_tx_buf, sizeof(gtp_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *)gtp_tx_buf;

	uint32_t seq_no = generate_rar_seq();

	upd_bearer_req_t ubr_req = {0};

	if (get_sess_entry_seid(pdn->seid, &context) != 0){
		clLog(clSystemLog, eCLSeverityCritical, "%s:%d NO Session Entry Found for sess ID:%lu\n",
				__func__, __LINE__, pdn->seid);
		return DIAMETER_ERROR_USER_UNKNOWN;
	}
    assert(pdn->context == context);
	/*VK : Start Creating UBR request */

	set_gtpv2c_teid_header((gtpv2c_header_t *) &ubr_req, GTP_UPDATE_BEARER_REQ,
	    										context->s11_mme_gtpc_teid, seq_no);

	ubr_req.apn_ambr.apn_ambr_uplnk = pdn->apn_ambr.ambr_uplink;
	ubr_req.apn_ambr.apn_ambr_dnlnk = pdn->apn_ambr.ambr_downlink;

	set_ie_header(&ubr_req.apn_ambr.header, GTP_IE_AGG_MAX_BIT_RATE, IE_INSTANCE_ZERO, sizeof(uint64_t));

	/* For now not supporting user location retrive
	set_ie_header(&ubr_req.indctn_flgs.header, GTP_IE_INDICATION, IE_INSTANCE_ZERO,
    	                           sizeof(gtp_indication_ie_t)- sizeof(ie_header_t));
	ubr_req.indctn_flgs.indication_retloc = 1;
	*/


	for (int32_t idx = 0; idx < pdn->policy.count ; idx++)
	{
		if (pdn->policy.pcc_rule[idx].action == RULE_ACTION_MODIFY)
		{

			bearer = get_bearer(pdn, &pdn->policy.pcc_rule[idx].dyn_rule->qos);
			if(bearer == NULL){
				clLog(sxlogger, eCLSeverityCritical,
						"%s:%d Bearer return is Null for that Qos recived in RAR: %d \n",
				 		__func__, __LINE__);
				return DIAMETER_ERROR_USER_UNKNOWN;

			}
			set_ie_header(&ubr_req.bearer_contexts[ubr_req.bearer_context_count].header,
												GTP_IE_BEARER_CONTEXT, IE_INSTANCE_ZERO, 0);


			int dyn_rule_count = bearer->num_dynamic_filters - 1;

			memset(bearer->dynamic_rules[dyn_rule_count], 0,
											sizeof(dynamic_rule_t));
			memcpy((bearer->dynamic_rules[dyn_rule_count]),
						(pdn->policy.pcc_rule[idx].dyn_rule),
										sizeof(dynamic_rule_t));

			len = set_bearer_tft(&ubr_req.bearer_contexts[ubr_req.bearer_context_count].tft,
									IE_INSTANCE_ZERO,
									bearer->dynamic_rules[dyn_rule_count]->num_flw_desc +
												TFT_REPLACE_FILTER_EXISTING - TFT_CREATE_NEW,
									bearer);
			ubr_req.bearer_contexts[ubr_req.bearer_context_count].header.len += len;
			update_require++;

#ifdef FUTURE_NEED 
			resp->list_bearer_ids[resp->num_of_bearers++] = bearer->eps_bearer_id;
#endif

			set_ebi(&ubr_req.bearer_contexts[ubr_req.bearer_context_count].eps_bearer_id,
					IE_INSTANCE_ZERO, bearer->eps_bearer_id);
			ubr_req.bearer_contexts[ubr_req.bearer_context_count].header.len += sizeof(uint8_t) + IE_HEADER_SIZE;

			++ubr_req.bearer_context_count;
			send_ubr++;

		}
	}

	gx_context_t *gx_context = NULL;
	/* Retrive Gx_context based on Sess ID. */
	ret = get_gx_context((uint8_t*)pdn->gx_sess_id, &gx_context);
	if (ret < 0) {
		clLog(clSystemLog, eCLSeverityCritical, "%s: NO ENTRY FOUND IN Gx HASH [%s]\n", __func__,
				pdn->gx_sess_id);
		return DIAMETER_UNKNOWN_SESSION_ID;
	}

	pdn->rqst_ptr = gx_context->rqst_ptr;


	/* Update UE State */
	pdn->state = UPDATE_BEARER_REQ_SNT_STATE;

	/* Update UE Proc */

#ifdef FUTURE_NEED
	pdn->proc = UPDATE_BEARER_PROC;
	/* Set GX rar message */
	resp->msg_type = GTP_UPDATE_BEARER_REQ;
	resp->state =  UPDATE_BEARER_REQ_SNT_STATE;
	resp->proc =  UPDATE_BEARER_PROC;
#endif


	if(send_ubr){
		uint16_t msg_len = 0;
		msg_len = encode_upd_bearer_req(&ubr_req, (uint8_t *)gtpv2c_tx);
		gtpv2c_tx->gtpc.message_len = htons(msg_len - 4);

		payload_length = ntohs(gtpv2c_tx->gtpc.message_len) + sizeof(gtpv2c_tx->gtpc);
		if(SAEGWC != cp_config->cp_type){
			//send S5S8 or on S11  interface update bearer request.
			gtpv2c_send(my_sock.sock_fd_s5s8, gtp_tx_buf, payload_length,
	    		      		(struct sockaddr *) &my_sock.s5s8_recv_sockaddr,
                            sizeof(struct sockaddr_in));
		}else{
            assert(0); 
#ifdef FUTURE_NEED
			s11_mme_sockaddr.sin_addr.s_addr =
								htonl(context->s11_mme_gtpc_ipv4.s_addr);
			gtpv2c_send(my_sock.sock_fd_s11, gtp_tx_buf, payload_length,
	    		      		(struct sockaddr *) &s11_mme_sockaddr,
	        				sizeof(struct sockaddr_in));
#endif
		}
	}
	return 0;
}


void
get_charging_rule_remove_bearer_info(pdn_connection_t *pdn,
	uint8_t *lbi, uint8_t *ded_ebi, uint8_t *ber_cnt)
{
	int8_t bearer_id;
	uint8_t idx_offset =  pdn->policy.num_charg_rule_install + pdn->policy.num_charg_rule_modify;

	for (int idx = 0; idx < pdn->policy.num_charg_rule_delete; idx++) {
		if(RULE_ACTION_DELETE == pdn->policy.pcc_rule[idx + idx_offset].action)
		{
			rule_name_key_t rule_name = {0};
			memcpy(&rule_name.rule_name, &(pdn->policy.pcc_rule[idx + idx_offset].dyn_rule->rule_name),
				   sizeof(pdn->policy.pcc_rule[idx + idx_offset].dyn_rule->rule_name));
			sprintf(rule_name.rule_name, "%s%d",
					rule_name.rule_name, pdn->call_id);

			bearer_id = get_rule_name_entry(rule_name);
			if (-1 == bearer_id) {
				/* TODO: Error handling bearer not found */
				return;
			}
			if (pdn->default_bearer_id == (bearer_id + 5)) {
				*lbi = pdn->default_bearer_id;
				*ber_cnt = pdn->num_bearer;
				for (int8_t iCnt = 0; iCnt < MAX_BEARERS; ++iCnt) {
					if (NULL != pdn->eps_bearers[iCnt]) {
						*ded_ebi = pdn->eps_bearers[iCnt]->eps_bearer_id;
						ded_ebi++;
					}
				}
				return;
			} else {
				*ded_ebi = bearer_id + 5;
				ded_ebi++;
				*ber_cnt = *ber_cnt + 1;
			}
		}
	}

	return;
}

int8_t
get_bearer_info_install_rules(pdn_connection_t *pdn, uint8_t *ebi)
{
	int8_t ret = 0;

	for (int idx = 0; idx < pdn->policy.num_charg_rule_install; idx++) {
		if(RULE_ACTION_ADD == pdn->policy.pcc_rule[idx].action)
		{
			rule_name_key_t rule_name = {0};
			memcpy(&rule_name.rule_name, &(pdn->policy.pcc_rule[idx].dyn_rule->rule_name),
				   sizeof(pdn->policy.pcc_rule[idx].dyn_rule->rule_name));
			sprintf(rule_name.rule_name, "%s%d",
					rule_name.rule_name, pdn->call_id);
			ret = get_rule_name_entry(rule_name);
			if (-1 == ret) {
				/* TODO: Error handling bearer not found */
				return ret;
			}

			*ebi = ret;

			return 0;
		}
	}

	return 0;
}
void pfcp_modify_rar_trigger_timeout(void *data)
{
    RTE_SET_USED(data);
    return;
}

int8_t
parse_gx_rar_msg(msg_info_t *msg)
{
    proc_context_t *proc_ctxt = msg->proc_context;
	int16_t ret = 0;
	pdn_connection_t *pdn_cntxt = proc_ctxt->pdn_context;
    ue_context_t *ue_context = proc_ctxt->ue_context;
	gx_context_t *gx_context = proc_ctxt->gx_context;
	pfcp_sess_mod_req_t pfcp_sess_mod_req = {0};

	//TODO :: Definfe generate_rar_seq
	int32_t seq_no = generate_rar_seq();

    /* PCRF Use case - Change in default bearer QoS through RAR */
    GxRAR *rar = &msg->gx_msg.rar;
	if(rar->presence.default_eps_bearer_qos)
	{
        // GXFIX - This is bug, we are not doing anything with this RAR request  
		ret = store_default_bearer_qos_in_policy(pdn_cntxt, rar->default_eps_bearer_qos);
		if (ret)
			return ret;
	}

    bool install_rule_present = (rar->presence.charging_rule_install == PRESENT);
    bool remove_rule_present = (rar->presence.charging_rule_remove == PRESENT);
	ret = store_dynamic_rules_in_policy(pdn_cntxt, &(rar->charging_rule_install), &(rar->charging_rule_remove), install_rule_present, remove_rule_present);
	if (ret) {
        printf("store_dynamic_rules_in_policy failed with return %d \n",ret);
	    return ret;
    }

    /* Requirement - updating policy only on either default bearer to dedicated bearer */
    printf("Modify pcc rules %d \n", pdn_cntxt->policy.num_charg_rule_modify);
	if(pdn_cntxt->policy.num_charg_rule_modify) {
        /*GXFIX : we are not updating pfcp session ? when should we do it ? */
        /* What if RAR is received which involves modify as well as addition of new rules, we can not 
         * just return from here...*/
		return gx_update_bearer_req(pdn_cntxt);
	}

	seq_no = fill_pfcp_gx_sess_mod_req(&pfcp_sess_mod_req, pdn_cntxt);

	uint8_t pfcp_msg[1024] = {0};
	int encoded = encode_pfcp_sess_mod_req_t(&pfcp_sess_mod_req, pfcp_msg);
	pfcp_header_t *header = (pfcp_header_t *) pfcp_msg;
	header->message_len = htons(encoded - 4);

	if ( pfcp_send(my_sock.sock_fd_pfcp, pfcp_msg, encoded, &ue_context->upf_context->upf_sockaddr) < 0 ){
		clLog(clSystemLog, eCLSeverityDebug,"Error sending: %i\n",errno);
	} else {
        increment_userplane_stats(MSG_TX_PFCP_SXASXB_SESSMODREQ, GET_UPF_ADDR(ue_context->upf_context));
    }
    transData_t *trans_entry = NULL;
    if(cp_config->cp_type == PGWC){
        trans_entry = start_response_wait_timer(ue_context,
                pfcp_msg, encoded,
                pfcp_modify_rar_trigger_timeout);
        RTE_SET_USED(trans_entry);
    }
    if(cp_config->cp_type == SAEGWC)
    {
        trans_entry = start_response_wait_timer(ue_context,
                pfcp_msg, encoded, pfcp_modify_rar_trigger_timeout);
    }
    uint32_t local_addr = my_sock.pfcp_sockaddr.sin_addr.s_addr;
    uint16_t port_num = my_sock.pfcp_sockaddr.sin_port;
    add_pfcp_transaction(local_addr, port_num, seq_no, (void*)trans_entry);
    trans_entry->sequence = seq_no;
    proc_ctxt->pfcp_trans = trans_entry;
    trans_entry->proc_context = proc_ctxt;
	/* Update UE State */
	pdn_cntxt->state = PFCP_SESS_MOD_REQ_SNT_STATE;

#ifdef FUTURE_NEED
	uint8_t bearer_id = 0;
	/* Set GX rar message */
	resp->eps_bearer_id = bearer_id + 5;
	resp->msg_type = GX_RAR_MSG;
	resp->state = PFCP_SESS_MOD_REQ_SNT_STATE;

	if(rar->charging_rule_remove.count != 0) {
		/* Update UE Proc */
		pdn_cntxt->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
		resp->proc = PDN_GW_INIT_BEARER_DEACTIVATION;
	} else {
		/* Update UE Proc */

		pdn_cntxt->proc = DED_BER_ACTIVATION_PROC;
		resp->proc = DED_BER_ACTIVATION_PROC;
	}
#endif

	pdn_cntxt->rqst_ptr = gx_context->rqst_ptr;

	return 0;
}
