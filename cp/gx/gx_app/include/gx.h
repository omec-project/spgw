// SPDX-FileCopyrightText: 2020-present Open Networking Foundation <info@opennetworking.org>
// Copyright (c) 2019 Sprint
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __GX_H__
#define __GX_H__

#include "fd.h"
#include "gx_struct.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
#define CONNECTPEER  "ConnectPeer"


#define ULI_EVENT_TRIGGER           13
#define UE_TIMEZONE_EVT_TRIGGER     25
#define ECGI_AND_TAI_PRESENT        17


/*The above flag will be set bit wise as
 *       * Bit 7| Bit 6 | Bit 5 | Bit 4 | Bit 3|  Bit 2|  Bit 1|  Bit 0 |
 *        *---------------------------------------------------------------
 *        *|     |       |       | ECGI  | RAI  |  SAI  |  CGI  |  TAI   |
 *        ----------------------------------------------------------------
 *
 */

#define ECGI_AND_TAI_PRESENT        17
#define TAI_PRESENT                 1
#define CGI_PRESENT                 2
#define SAI_PRESENT                 4
#define RAI_PRESENT                 8
#define ECGI_PRESENT                16


#define GX_UE_TIMEZONE_TYPE         0x17
#define GX_ECGI_AND_TAI_TYPE        0x82
#define GX_TAI_TYPE                 0x80
#define GX_ECGI_TYPE                0x81
#define GX_SAI_TYPE                 0x01
#define GX_RAI_TYPE                 0x02
#define GX_CGI_TYPE                 0x00



enum diameter_error {
	DIAMETER_UNKNOWN_SESSION_ID = 5002,
	DIAMETER_ERROR_USER_UNKNOWN = 5030,
	DIAMETER_UNABLE_TO_COMPLY =  5012
};

/**
 * @brief  : Maintains gx dictionary information
 */
typedef struct gxDict {
    struct dict_object * vndETSI;
    struct dict_object * vnd3GPP2;
    struct dict_object * vnd3GPP;
    struct dict_object * appGX;


	struct dict_application_data davp_appGX;
	struct dict_vendor_data davp_vndETSI;
	struct dict_vendor_data davp_vnd3GPP2;
	struct dict_vendor_data davp_vnd3GPP;

    struct dict_object * cmdRAR;
    struct dict_object * cmdRAA;
    struct dict_object * cmdCCA;
    struct dict_object * cmdCCR;

    struct dict_object * avp_tdf_destination_host;
    struct dict_object * avp_pre_emption_capability;
    struct dict_object * avp_packet_filter_content;
    struct dict_object * avp_feature_list_id;
    struct dict_object * avp_resource_release_notification;
    struct dict_object * avp_service_identifier;
    struct dict_object * avp_physical_access_id;
    struct dict_object * avp_csg_access_mode;
    struct dict_object * avp_henb_local_ip_address;
    struct dict_object * avp_dynamic_address_flag_extension;
    struct dict_object * avp_apn_aggregate_max_bitrate_ul;
    struct dict_object * avp_network_request_support;
    struct dict_object * avp_termination_cause;
    struct dict_object * avp_exponent;
    struct dict_object * avp_3gpp_rat_type;
    struct dict_object * avp_af_signalling_protocol;
    struct dict_object * avp_packet_filter_usage;
    struct dict_object * avp_usage_monitoring_support;
    struct dict_object * avp_tracking_area_identity;
    struct dict_object * avp_load_value;
    struct dict_object * avp_feature_list;
    struct dict_object * avp_omc_id;
    struct dict_object * avp_rai;
    struct dict_object * avp_oc_report_type;
    struct dict_object * avp_experimental_result;
    struct dict_object * avp_cc_request_type;
    struct dict_object * avp_service_context_id;
    struct dict_object * avp_secondary_event_charging_function_name;
    struct dict_object * avp_pcscf_restoration_indication;
    struct dict_object * avp_tdf_ip_address;
    struct dict_object * avp_load_type;
    struct dict_object * avp_extended_gbr_dl;
    struct dict_object * avp_oc_feature_vector;
    struct dict_object * avp_origin_host;
    struct dict_object * avp_pra_remove;
    struct dict_object * avp_maximum_wait_time;
    struct dict_object * avp_list_of_measurements;
    struct dict_object * avp_qos_information;
    struct dict_object * avp_final_unit_action;
    struct dict_object * avp_conditional_policy_information;
    struct dict_object * avp_drmp;
    struct dict_object * avp_pra_install;
    struct dict_object * avp_logical_access_id;
    struct dict_object * avp_resource_allocation_notification;
    struct dict_object * avp_rule_deactivation_time;
    struct dict_object * avp_flow_status;
    struct dict_object * avp_priority_level;
    struct dict_object * avp_pre_emption_vulnerability;
    struct dict_object * avp_bearer_usage;
    struct dict_object * avp_reporting_reason;
    struct dict_object * avp_qos_class_identifier;
    struct dict_object * avp_3gpp_sgsn_mcc_mnc;
    struct dict_object * avp_area_scope;
    struct dict_object * avp_re_auth_request_type;
    struct dict_object * avp_precedence;
    struct dict_object * avp_flow_number;
    struct dict_object * avp_pdn_connection_charging_id;
    struct dict_object * avp_3gpp_ps_data_off_status;
    struct dict_object * avp_redirect_host_usage;
    struct dict_object * avp_an_gw_address;
    struct dict_object * avp_tunnel_header_filter;
    struct dict_object * avp_access_network_charging_identifier_value;
    struct dict_object * avp_secondary_charging_collection_function_name;
    struct dict_object * avp_tcp_source_port;
    struct dict_object * avp_destination_host;
    struct dict_object * avp_3gpp_selection_mode;
    struct dict_object * avp_location_area_identity;
    struct dict_object * avp_logging_interval;
    struct dict_object * avp_flow_information;
    struct dict_object * avp_ue_local_ip_address;
    struct dict_object * avp_extended_apn_ambr_dl;
    struct dict_object * avp_tdf_application_identifier;
    struct dict_object * avp_tunnel_information;
    struct dict_object * avp_media_component_status;
    struct dict_object * avp_tft_packet_filter_information;
    struct dict_object * avp_guaranteed_bitrate_ul;
    struct dict_object * avp_online;
    struct dict_object * avp_mbsfn_area;
    struct dict_object * avp_extended_apn_ambr_ul;
    struct dict_object * avp_extended_gbr_ul;
    struct dict_object * avp_content_version;
    struct dict_object * avp_usage_monitoring_report;
    struct dict_object * avp_event_report_indication;
    struct dict_object * avp_job_type;
    struct dict_object * avp_bearer_operation;
    struct dict_object * avp_user_equipment_info_type;
    struct dict_object * avp_tdf_information;
    struct dict_object * avp_cc_request_number;
    struct dict_object * avp_framed_ipv6_prefix;
    struct dict_object * avp_packet_filter_operation;
    struct dict_object * avp_coa_ip_address;
    struct dict_object * avp_3gpp_charging_characteristics;
    struct dict_object * avp_proxy_info;
    struct dict_object * avp_used_service_unit;
    struct dict_object * avp_charging_rule_install;
    struct dict_object * avp_mdt_allowed_plmn_id;
    struct dict_object * avp_origin_realm;
    struct dict_object * avp_twan_identifier;
    struct dict_object * avp_charging_rule_definition;
    struct dict_object * avp_flow_label;
    struct dict_object * avp_3gpp_ggsn_ipv6_address;
    struct dict_object * avp_guaranteed_bitrate_dl;
    struct dict_object * avp_restriction_filter_rule;
    struct dict_object * avp_3gpp_sgsn_address;
    struct dict_object * avp_redirect_address_type;
    struct dict_object * avp_tdf_destination_realm;
    struct dict_object * avp_user_location_info_time;
    struct dict_object * avp_subscription_id_data;
    struct dict_object * avp_redirect_server_address;
    struct dict_object * avp_nbifom_mode;
    struct dict_object * avp_final_unit_indication;
    struct dict_object * avp_3gpp_sgsn_ipv6_address;
    struct dict_object * avp_3gpp2_bsid;
    struct dict_object * avp_trace_collection_entity;
    struct dict_object * avp_session_release_cause;
    struct dict_object * avp_ran_rule_support;
    struct dict_object * avp_unit_value;
    struct dict_object * avp_charging_rule_base_name;
    struct dict_object * avp_report_interval;
    struct dict_object * avp_presence_reporting_area_node;
    struct dict_object * avp_user_equipment_info_value;
    struct dict_object * avp_route_record;
    struct dict_object * avp_presence_reporting_area_identifier;
    struct dict_object * avp_csg_information_reporting;
    struct dict_object * avp_filter_id;
    struct dict_object * avp_presence_reporting_area_information;
    struct dict_object * avp_an_gw_status;
    struct dict_object * avp_ssid;
    struct dict_object * avp_metering_method;
    struct dict_object * avp_flow_description;
    struct dict_object * avp_logging_duration;
    struct dict_object * avp_apn_aggregate_max_bitrate_dl;
    struct dict_object * avp_conditional_apn_aggregate_max_bitrate;
    struct dict_object * avp_access_network_charging_identifier_gx;
    struct dict_object * avp_positioning_method;
    struct dict_object * avp_oc_olr;
    struct dict_object * avp_routing_rule_install;
    struct dict_object * avp_presence_reporting_area_status;
    struct dict_object * avp_trace_data;
    struct dict_object * avp_sourceid;
    struct dict_object * avp_carrier_frequency;
    struct dict_object * avp_mbsfn_area_id;
    struct dict_object * avp_subscription_id_type;
    struct dict_object * avp_usage_monitoring_level;
    struct dict_object * avp_bearer_identifier;
    struct dict_object * avp_sponsor_identity;
    struct dict_object * avp_oc_reduction_percentage;
    struct dict_object * avp_default_qos_name;
    struct dict_object * avp_routing_rule_definition;
    struct dict_object * avp_traffic_steering_policy_identifier_ul;
    struct dict_object * avp_mdt_configuration;
    struct dict_object * avp_error_reporting_host;
    struct dict_object * avp_charging_rule_remove;
    struct dict_object * avp_charging_correlation_indicator;
    struct dict_object * avp_nbifom_support;
    struct dict_object * avp_max_plr_dl;
    struct dict_object * avp_event_threshold_event_1i;
    struct dict_object * avp_rating_group;
    struct dict_object * avp_rat_type;
    struct dict_object * avp_event_charging_timestamp;
    struct dict_object * avp_default_access;
    struct dict_object * avp_event_threshold_event_1f;
    struct dict_object * avp_reporting_level;
    struct dict_object * avp_allocation_retention_priority;
    struct dict_object * avp_bearer_control_mode;
    struct dict_object * avp_cell_global_identity;
    struct dict_object * avp_max_plr_ul;
    struct dict_object * avp_oc_validity_duration;
    struct dict_object * avp_application_service_provider_identity;
    struct dict_object * avp_csg_membership_indication;
    struct dict_object * avp_flow_direction;
    struct dict_object * avp_sharing_key_dl;
    struct dict_object * avp_default_eps_bearer_qos;
    struct dict_object * avp_trace_ne_type_list;
    struct dict_object * avp_extended_max_requested_bw_dl;
    struct dict_object * avp_redirect_host;
    struct dict_object * avp_measurement_period_lte;
    struct dict_object * avp_routing_rule_report;
    struct dict_object * avp_max_requested_bandwidth_dl;
    struct dict_object * avp_user_equipment_info;
    struct dict_object * avp_quota_consumption_time;
    struct dict_object * avp_origin_state_id;
    struct dict_object * avp_qos_negotiation;
    struct dict_object * avp_cc_output_octets;
    struct dict_object * avp_ran_nas_release_cause;
    struct dict_object * avp_sharing_key_ul;
    struct dict_object * avp_netloc_access_support;
    struct dict_object * avp_trace_event_list;
    struct dict_object * avp_supported_features;
    struct dict_object * avp_3gpp_user_location_info;
    struct dict_object * avp_value_digits;
    struct dict_object * avp_security_parameter_index;
    struct dict_object * avp_result_code;
    struct dict_object * avp_trace_interface_list;
    struct dict_object * avp_fixed_user_location_info;
    struct dict_object * avp_default_qos_information;
    struct dict_object * avp_traffic_steering_policy_identifier_dl;
    struct dict_object * avp_redirect_max_cache_time;
    struct dict_object * avp_rule_activation_time;
    struct dict_object * avp_load;
    struct dict_object * avp_3gpp_ggsn_address;
    struct dict_object * avp_redirect_server;
    struct dict_object * avp_an_trusted;
    struct dict_object * avp_e_utran_cell_global_identity;
    struct dict_object * avp_called_station_id;
    struct dict_object * avp_csg_id;
    struct dict_object * avp_framed_ip_address;
    struct dict_object * avp_oc_supported_features;
    struct dict_object * avp_packet_filter_identifier;
    struct dict_object * avp_pcc_rule_status;
    struct dict_object * avp_tdf_application_instance_identifier;
    struct dict_object * avp_proxy_host;
    struct dict_object * avp_event_threshold_rsrp;
    struct dict_object * avp_event_threshold_rsrq;
    struct dict_object * avp_packet_filter_information;
    struct dict_object * avp_subscription_id;
    struct dict_object * avp_experimental_result_code;
    struct dict_object * avp_collection_period_rrm_lte;
    struct dict_object * avp_pdn_connection_id;
    struct dict_object * avp_access_network_charging_address;
    struct dict_object * avp_auth_application_id;
    struct dict_object * avp_revalidation_time;
    struct dict_object * avp_execution_time;
    struct dict_object * avp_event_trigger;
    struct dict_object * avp_extended_max_requested_bw_ul;
    struct dict_object * avp_presence_reporting_area_elements_list;
    struct dict_object * avp_charging_information;
    struct dict_object * avp_monitoring_key;
    struct dict_object * avp_3gpp_ms_timezone;
    struct dict_object * avp_charging_rule_name;
    struct dict_object * avp_access_availability_change_reason;
    struct dict_object * avp_dynamic_address_flag;
    struct dict_object * avp_monitoring_flags;
    struct dict_object * avp_collection_period_rrm_umts;
    struct dict_object * avp_usage_monitoring_information;
    struct dict_object * avp_charging_rule_report;
    struct dict_object * avp_ip_can_type;
    struct dict_object * avp_offline;
    struct dict_object * avp_udp_source_port;
    struct dict_object * avp_routing_ip_address;
    struct dict_object * avp_redirect_information;
    struct dict_object * avp_mute_notification;
    struct dict_object * avp_media_component_number;
    struct dict_object * avp_tariff_time_change;
    struct dict_object * avp_error_message;
    struct dict_object * avp_credit_management_status;
    struct dict_object * avp_required_access_info;
    struct dict_object * avp_ip_can_session_charging_scope;
    struct dict_object * avp_reporting_trigger;
    struct dict_object * avp_failed_avp;
    struct dict_object * avp_routing_area_identity;
    struct dict_object * avp_routing_rule_remove;
    struct dict_object * avp_tft_filter;
    struct dict_object * avp_trace_reference;
    struct dict_object * avp_cc_service_specific_units;
    struct dict_object * avp_cc_time;
    struct dict_object * avp_currency_code;
    struct dict_object * avp_cc_input_octets;
    struct dict_object * avp_measurement_quantity;
    struct dict_object * avp_removal_of_access;
    struct dict_object * avp_routing_filter;
    struct dict_object * avp_trace_depth;
    struct dict_object * avp_proxy_state;
    struct dict_object * avp_rule_failure_code;
    struct dict_object * avp_af_charging_identifier;
    struct dict_object * avp_tunnel_header_length;
    struct dict_object * avp_routing_rule_failure_code;
    struct dict_object * avp_coa_information;
    struct dict_object * avp_default_bearer_indication;
    struct dict_object * avp_vendor_id;
    struct dict_object * avp_granted_service_unit;
    struct dict_object * avp_max_requested_bandwidth_ul;
    struct dict_object * avp_oc_sequence_number;
    struct dict_object * avp_routing_rule_identifier;
    struct dict_object * avp_redirect_support;
    struct dict_object * avp_destination_realm;
    struct dict_object * avp_session_id;
    struct dict_object * avp_tos_traffic_class;
    struct dict_object * avp_origination_time_stamp;
    struct dict_object * avp_bssid;
    struct dict_object * avp_cc_money;
    struct dict_object * avp_application_detection_information;
    struct dict_object * avp_qos_upgrade;
    struct dict_object * avp_tariff_change_usage;
    struct dict_object * avp_report_amount;
    struct dict_object * avp_primary_event_charging_function_name;
    struct dict_object * avp_cc_total_octets;
    struct dict_object * avp_measurement_period_umts;
    struct dict_object * avp_flows;
    struct dict_object * avp_ps_to_cs_session_continuity;
    struct dict_object * avp_primary_charging_collection_function_name;
    struct dict_object * avp_user_csg_information;

    struct dict_avp_data davp_tdf_destination_host;
    struct dict_avp_data davp_pre_emption_capability;
    struct dict_avp_data davp_packet_filter_content;
    struct dict_avp_data davp_feature_list_id;
    struct dict_avp_data davp_resource_release_notification;
    struct dict_avp_data davp_service_identifier;
    struct dict_avp_data davp_physical_access_id;
    struct dict_avp_data davp_csg_access_mode;
    struct dict_avp_data davp_henb_local_ip_address;
    struct dict_avp_data davp_dynamic_address_flag_extension;
    struct dict_avp_data davp_apn_aggregate_max_bitrate_ul;
    struct dict_avp_data davp_network_request_support;
    struct dict_avp_data davp_termination_cause;
    struct dict_avp_data davp_exponent;
    struct dict_avp_data davp_3gpp_rat_type;
    struct dict_avp_data davp_af_signalling_protocol;
    struct dict_avp_data davp_packet_filter_usage;
    struct dict_avp_data davp_usage_monitoring_support;
    struct dict_avp_data davp_tracking_area_identity;
    struct dict_avp_data davp_load_value;
    struct dict_avp_data davp_feature_list;
    struct dict_avp_data davp_omc_id;
    struct dict_avp_data davp_rai;
    struct dict_avp_data davp_oc_report_type;
    struct dict_avp_data davp_experimental_result;
    struct dict_avp_data davp_cc_request_type;
    struct dict_avp_data davp_service_context_id;
    struct dict_avp_data davp_secondary_event_charging_function_name;
    struct dict_avp_data davp_pcscf_restoration_indication;
    struct dict_avp_data davp_tdf_ip_address;
    struct dict_avp_data davp_load_type;
    struct dict_avp_data davp_extended_gbr_dl;
    struct dict_avp_data davp_oc_feature_vector;
    struct dict_avp_data davp_origin_host;
    struct dict_avp_data davp_pra_remove;
    struct dict_avp_data davp_maximum_wait_time;
    struct dict_avp_data davp_list_of_measurements;
    struct dict_avp_data davp_qos_information;
    struct dict_avp_data davp_final_unit_action;
    struct dict_avp_data davp_conditional_policy_information;
    struct dict_avp_data davp_drmp;
    struct dict_avp_data davp_pra_install;
    struct dict_avp_data davp_logical_access_id;
    struct dict_avp_data davp_resource_allocation_notification;
    struct dict_avp_data davp_rule_deactivation_time;
    struct dict_avp_data davp_flow_status;
    struct dict_avp_data davp_priority_level;
    struct dict_avp_data davp_pre_emption_vulnerability;
    struct dict_avp_data davp_bearer_usage;
    struct dict_avp_data davp_reporting_reason;
    struct dict_avp_data davp_qos_class_identifier;
    struct dict_avp_data davp_3gpp_sgsn_mcc_mnc;
    struct dict_avp_data davp_area_scope;
    struct dict_avp_data davp_re_auth_request_type;
    struct dict_avp_data davp_precedence;
    struct dict_avp_data davp_flow_number;
    struct dict_avp_data davp_pdn_connection_charging_id;
    struct dict_avp_data davp_3gpp_ps_data_off_status;
    struct dict_avp_data davp_redirect_host_usage;
    struct dict_avp_data davp_an_gw_address;
    struct dict_avp_data davp_tunnel_header_filter;
    struct dict_avp_data davp_access_network_charging_identifier_value;
    struct dict_avp_data davp_secondary_charging_collection_function_name;
    struct dict_avp_data davp_tcp_source_port;
    struct dict_avp_data davp_destination_host;
    struct dict_avp_data davp_3gpp_selection_mode;
    struct dict_avp_data davp_location_area_identity;
    struct dict_avp_data davp_logging_interval;
    struct dict_avp_data davp_flow_information;
    struct dict_avp_data davp_ue_local_ip_address;
    struct dict_avp_data davp_extended_apn_ambr_dl;
    struct dict_avp_data davp_tdf_application_identifier;
    struct dict_avp_data davp_tunnel_information;
    struct dict_avp_data davp_media_component_status;
    struct dict_avp_data davp_tft_packet_filter_information;
    struct dict_avp_data davp_guaranteed_bitrate_ul;
    struct dict_avp_data davp_online;
    struct dict_avp_data davp_mbsfn_area;
    struct dict_avp_data davp_extended_apn_ambr_ul;
    struct dict_avp_data davp_extended_gbr_ul;
    struct dict_avp_data davp_content_version;
    struct dict_avp_data davp_usage_monitoring_report;
    struct dict_avp_data davp_event_report_indication;
    struct dict_avp_data davp_job_type;
    struct dict_avp_data davp_bearer_operation;
    struct dict_avp_data davp_user_equipment_info_type;
    struct dict_avp_data davp_tdf_information;
    struct dict_avp_data davp_cc_request_number;
    struct dict_avp_data davp_framed_ipv6_prefix;
    struct dict_avp_data davp_packet_filter_operation;
    struct dict_avp_data davp_coa_ip_address;
    struct dict_avp_data davp_3gpp_charging_characteristics;
    struct dict_avp_data davp_proxy_info;
    struct dict_avp_data davp_used_service_unit;
    struct dict_avp_data davp_charging_rule_install;
    struct dict_avp_data davp_mdt_allowed_plmn_id;
    struct dict_avp_data davp_origin_realm;
    struct dict_avp_data davp_twan_identifier;
    struct dict_avp_data davp_charging_rule_definition;
    struct dict_avp_data davp_flow_label;
    struct dict_avp_data davp_3gpp_ggsn_ipv6_address;
    struct dict_avp_data davp_guaranteed_bitrate_dl;
    struct dict_avp_data davp_restriction_filter_rule;
    struct dict_avp_data davp_3gpp_sgsn_address;
    struct dict_avp_data davp_redirect_address_type;
    struct dict_avp_data davp_tdf_destination_realm;
    struct dict_avp_data davp_user_location_info_time;
    struct dict_avp_data davp_subscription_id_data;
    struct dict_avp_data davp_redirect_server_address;
    struct dict_avp_data davp_nbifom_mode;
    struct dict_avp_data davp_final_unit_indication;
    struct dict_avp_data davp_3gpp_sgsn_ipv6_address;
    struct dict_avp_data davp_3gpp2_bsid;
    struct dict_avp_data davp_trace_collection_entity;
    struct dict_avp_data davp_session_release_cause;
    struct dict_avp_data davp_ran_rule_support;
    struct dict_avp_data davp_unit_value;
    struct dict_avp_data davp_charging_rule_base_name;
    struct dict_avp_data davp_report_interval;
    struct dict_avp_data davp_presence_reporting_area_node;
    struct dict_avp_data davp_user_equipment_info_value;
    struct dict_avp_data davp_route_record;
    struct dict_avp_data davp_presence_reporting_area_identifier;
    struct dict_avp_data davp_csg_information_reporting;
    struct dict_avp_data davp_filter_id;
    struct dict_avp_data davp_presence_reporting_area_information;
    struct dict_avp_data davp_an_gw_status;
    struct dict_avp_data davp_ssid;
    struct dict_avp_data davp_metering_method;
    struct dict_avp_data davp_flow_description;
    struct dict_avp_data davp_logging_duration;
    struct dict_avp_data davp_apn_aggregate_max_bitrate_dl;
    struct dict_avp_data davp_conditional_apn_aggregate_max_bitrate;
    struct dict_avp_data davp_access_network_charging_identifier_gx;
    struct dict_avp_data davp_positioning_method;
    struct dict_avp_data davp_oc_olr;
    struct dict_avp_data davp_routing_rule_install;
    struct dict_avp_data davp_presence_reporting_area_status;
    struct dict_avp_data davp_trace_data;
    struct dict_avp_data davp_sourceid;
    struct dict_avp_data davp_carrier_frequency;
    struct dict_avp_data davp_mbsfn_area_id;
    struct dict_avp_data davp_subscription_id_type;
    struct dict_avp_data davp_usage_monitoring_level;
    struct dict_avp_data davp_bearer_identifier;
    struct dict_avp_data davp_sponsor_identity;
    struct dict_avp_data davp_oc_reduction_percentage;
    struct dict_avp_data davp_default_qos_name;
    struct dict_avp_data davp_routing_rule_definition;
    struct dict_avp_data davp_traffic_steering_policy_identifier_ul;
    struct dict_avp_data davp_mdt_configuration;
    struct dict_avp_data davp_error_reporting_host;
    struct dict_avp_data davp_charging_rule_remove;
    struct dict_avp_data davp_charging_correlation_indicator;
    struct dict_avp_data davp_nbifom_support;
    struct dict_avp_data davp_max_plr_dl;
    struct dict_avp_data davp_event_threshold_event_1i;
    struct dict_avp_data davp_rating_group;
    struct dict_avp_data davp_rat_type;
    struct dict_avp_data davp_event_charging_timestamp;
    struct dict_avp_data davp_default_access;
    struct dict_avp_data davp_event_threshold_event_1f;
    struct dict_avp_data davp_reporting_level;
    struct dict_avp_data davp_allocation_retention_priority;
    struct dict_avp_data davp_bearer_control_mode;
    struct dict_avp_data davp_cell_global_identity;
    struct dict_avp_data davp_max_plr_ul;
    struct dict_avp_data davp_oc_validity_duration;
    struct dict_avp_data davp_application_service_provider_identity;
    struct dict_avp_data davp_csg_membership_indication;
    struct dict_avp_data davp_flow_direction;
    struct dict_avp_data davp_sharing_key_dl;
    struct dict_avp_data davp_default_eps_bearer_qos;
    struct dict_avp_data davp_trace_ne_type_list;
    struct dict_avp_data davp_extended_max_requested_bw_dl;
    struct dict_avp_data davp_redirect_host;
    struct dict_avp_data davp_measurement_period_lte;
    struct dict_avp_data davp_routing_rule_report;
    struct dict_avp_data davp_max_requested_bandwidth_dl;
    struct dict_avp_data davp_user_equipment_info;
    struct dict_avp_data davp_quota_consumption_time;
    struct dict_avp_data davp_origin_state_id;
    struct dict_avp_data davp_qos_negotiation;
    struct dict_avp_data davp_cc_output_octets;
    struct dict_avp_data davp_ran_nas_release_cause;
    struct dict_avp_data davp_sharing_key_ul;
    struct dict_avp_data davp_netloc_access_support;
    struct dict_avp_data davp_trace_event_list;
    struct dict_avp_data davp_supported_features;
    struct dict_avp_data davp_3gpp_user_location_info;
    struct dict_avp_data davp_value_digits;
    struct dict_avp_data davp_security_parameter_index;
    struct dict_avp_data davp_result_code;
    struct dict_avp_data davp_trace_interface_list;
    struct dict_avp_data davp_fixed_user_location_info;
    struct dict_avp_data davp_default_qos_information;
    struct dict_avp_data davp_traffic_steering_policy_identifier_dl;
    struct dict_avp_data davp_redirect_max_cache_time;
    struct dict_avp_data davp_rule_activation_time;
    struct dict_avp_data davp_load;
    struct dict_avp_data davp_3gpp_ggsn_address;
    struct dict_avp_data davp_redirect_server;
    struct dict_avp_data davp_an_trusted;
    struct dict_avp_data davp_e_utran_cell_global_identity;
    struct dict_avp_data davp_called_station_id;
    struct dict_avp_data davp_csg_id;
    struct dict_avp_data davp_framed_ip_address;
    struct dict_avp_data davp_oc_supported_features;
    struct dict_avp_data davp_packet_filter_identifier;
    struct dict_avp_data davp_pcc_rule_status;
    struct dict_avp_data davp_tdf_application_instance_identifier;
    struct dict_avp_data davp_proxy_host;
    struct dict_avp_data davp_event_threshold_rsrp;
    struct dict_avp_data davp_event_threshold_rsrq;
    struct dict_avp_data davp_packet_filter_information;
    struct dict_avp_data davp_subscription_id;
    struct dict_avp_data davp_experimental_result_code;
    struct dict_avp_data davp_collection_period_rrm_lte;
    struct dict_avp_data davp_pdn_connection_id;
    struct dict_avp_data davp_access_network_charging_address;
    struct dict_avp_data davp_auth_application_id;
    struct dict_avp_data davp_revalidation_time;
    struct dict_avp_data davp_execution_time;
    struct dict_avp_data davp_event_trigger;
    struct dict_avp_data davp_extended_max_requested_bw_ul;
    struct dict_avp_data davp_presence_reporting_area_elements_list;
    struct dict_avp_data davp_charging_information;
    struct dict_avp_data davp_monitoring_key;
    struct dict_avp_data davp_3gpp_ms_timezone;
    struct dict_avp_data davp_charging_rule_name;
    struct dict_avp_data davp_access_availability_change_reason;
    struct dict_avp_data davp_dynamic_address_flag;
    struct dict_avp_data davp_monitoring_flags;
    struct dict_avp_data davp_collection_period_rrm_umts;
    struct dict_avp_data davp_usage_monitoring_information;
    struct dict_avp_data davp_charging_rule_report;
    struct dict_avp_data davp_ip_can_type;
    struct dict_avp_data davp_offline;
    struct dict_avp_data davp_udp_source_port;
    struct dict_avp_data davp_routing_ip_address;
    struct dict_avp_data davp_redirect_information;
    struct dict_avp_data davp_mute_notification;
    struct dict_avp_data davp_media_component_number;
    struct dict_avp_data davp_tariff_time_change;
    struct dict_avp_data davp_error_message;
    struct dict_avp_data davp_credit_management_status;
    struct dict_avp_data davp_required_access_info;
    struct dict_avp_data davp_ip_can_session_charging_scope;
    struct dict_avp_data davp_reporting_trigger;
    struct dict_avp_data davp_failed_avp;
    struct dict_avp_data davp_routing_area_identity;
    struct dict_avp_data davp_routing_rule_remove;
    struct dict_avp_data davp_tft_filter;
    struct dict_avp_data davp_trace_reference;
    struct dict_avp_data davp_cc_service_specific_units;
    struct dict_avp_data davp_cc_time;
    struct dict_avp_data davp_currency_code;
    struct dict_avp_data davp_cc_input_octets;
    struct dict_avp_data davp_measurement_quantity;
    struct dict_avp_data davp_removal_of_access;
    struct dict_avp_data davp_routing_filter;
    struct dict_avp_data davp_trace_depth;
    struct dict_avp_data davp_proxy_state;
    struct dict_avp_data davp_rule_failure_code;
    struct dict_avp_data davp_af_charging_identifier;
    struct dict_avp_data davp_tunnel_header_length;
    struct dict_avp_data davp_routing_rule_failure_code;
    struct dict_avp_data davp_coa_information;
    struct dict_avp_data davp_default_bearer_indication;
    struct dict_avp_data davp_vendor_id;
    struct dict_avp_data davp_granted_service_unit;
    struct dict_avp_data davp_max_requested_bandwidth_ul;
    struct dict_avp_data davp_oc_sequence_number;
    struct dict_avp_data davp_routing_rule_identifier;
    struct dict_avp_data davp_redirect_support;
    struct dict_avp_data davp_destination_realm;
    struct dict_avp_data davp_session_id;
    struct dict_avp_data davp_tos_traffic_class;
    struct dict_avp_data davp_origination_time_stamp;
    struct dict_avp_data davp_bssid;
    struct dict_avp_data davp_cc_money;
    struct dict_avp_data davp_application_detection_information;
    struct dict_avp_data davp_qos_upgrade;
    struct dict_avp_data davp_tariff_change_usage;
    struct dict_avp_data davp_report_amount;
    struct dict_avp_data davp_primary_event_charging_function_name;
    struct dict_avp_data davp_cc_total_octets;
    struct dict_avp_data davp_measurement_period_umts;
    struct dict_avp_data davp_flows;
    struct dict_avp_data davp_ps_to_cs_session_continuity;
    struct dict_avp_data davp_primary_charging_collection_function_name;
    struct dict_avp_data davp_user_csg_information;
} GxDict;

extern GxDict gxDict;

extern int gxInit     (void);
extern int gxRegister (void);

extern int gx_rar_cb (struct msg ** msg, struct avp * avp, struct session * sess, void * data, enum disp_action * act);
extern int gx_raa_cb (struct msg ** msg, struct avp * avp, struct session * sess, void * data, enum disp_action * act);
extern int gx_cca_cb (struct msg ** msg, struct avp * avp, struct session * sess, void * data, enum disp_action * act);
extern int gx_ccr_cb (struct msg ** msg, struct avp * avp, struct session * sess, void * data, enum disp_action * act);

extern int gx_send_rar (void *data);
extern int gx_send_ccr (void *data);

extern int gx_rar_parse(struct msg * msg, GxRAR *rar);
extern int gx_raa_parse(struct msg * msg, GxRAA *raa);
extern int gx_cca_parse(struct msg * msg, GxCCA *cca);
extern int gx_ccr_parse(struct msg * msg, GxCCR *ccr);

extern int gx_rar_free (GxRAR *rar);
extern int gx_raa_free (GxRAA *raa);
extern int gx_cca_free (GxCCA *cca);
extern int gx_ccr_free (GxCCR *ccr);

extern uint32_t gx_rar_calc_length (GxRAR *rar);
extern uint32_t gx_raa_calc_length (GxRAA *raa);
extern uint32_t gx_cca_calc_length (GxCCA *cca);
extern uint32_t gx_ccr_calc_length (GxCCR *ccr);

extern int gx_rar_pack (GxRAR *rar, unsigned char *buf, uint32_t buflen);
extern int gx_raa_pack (GxRAA *raa, unsigned char *buf, uint32_t buflen);
extern int gx_cca_pack (GxCCA *cca, unsigned char *buf, uint32_t buflen);
extern int gx_ccr_pack (GxCCR *ccr, unsigned char *buf, uint32_t buflen);

extern int gx_rar_unpack (unsigned char *buf, GxRAR *rar);
extern int gx_raa_unpack (unsigned char *buf, GxRAA *raa);
extern int gx_cca_unpack (unsigned char *buf, GxCCA *cca);
extern int gx_ccr_unpack (unsigned char *buf, GxCCR *ccr);

int gx_send_raa(void *data);

int recv_msg_handler( int sock );
int unixsock(void);
// placeholder for transactio 
struct pending_gx_trans{
    void *msg;
    uint16_t seq_num;
    LIST_ENTRY(pending_gx_trans) next_trans_entry;
};
typedef struct pending_gx_trans pending_gx_trans_t;


struct gx_trans_data {
   uint16_t rar_seq_num;
   LIST_HEAD(pending_gx_trans_head, pending_gx_trans) pending_gx_trans; 
};
typedef struct gx_trans_data gx_trans_data_t;

#ifdef __cplusplus
}
#endif
#endif /* __GX_H__ */
