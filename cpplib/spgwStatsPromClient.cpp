/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>
#include <chrono>
#include <map>
#include <memory>
#include <thread>
#include <sstream>
#include "spgwStatsPromClient.h"

using namespace prometheus;
std::shared_ptr<Registry> registry;

void spgwStatsSetupPrometheusThread(uint16_t port)
{
    std::stringstream ss;
    ss << "0.0.0.0"<<":"<<port;
    registry = std::make_shared<Registry>();
    /* Create single instance */ 
    spgwStats::Instance(); 
    Exposer exposer{ss.str(), 1};
    std::string metrics("/metrics");
    exposer.RegisterCollectable(registry, metrics);
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
spgwStats::spgwStats()
{
	 num_ue_m = new num_ue_gauges;
	 data_usage_m = new data_usage_gauges;
	 subscribers_info_m = new subscribers_info_gauges;
	 msg_rx_m = new msg_rx_counters;
	 msg_tx_m = new msg_tx_counters;
	 procedures_m = new procedures_counters;
}
spgwStats* spgwStats::Instance() 
{
	static spgwStats object;
	return &object; 
}


num_ue_gauges::num_ue_gauges():
num_ue_family(BuildGauge().Name("spgw_number_of_ue_attached").Help("Number of UE attached").Labels({{"spgw_num_ue","subscribers"}}).Register(*registry)),
current__spgw_active_subscribers(num_ue_family.Add({{"cp_mode","spgw"},{"state","active"},{"level","subscribers"}})),
current__spgw_idle_subscribers(num_ue_family.Add({{"cp_mode","spgw"},{"state","idle"},{"level","subscribers"}})),
current__pgw_active_subscribers(num_ue_family.Add({{"cp_mode","pgw"},{"state","active"},{"level","subscribers"}})),
current__pgw_idle_subscribers(num_ue_family.Add({{"cp_mode","pgw"},{"state","idle"},{"level","subscribers"}})),
current__spgw_active_pdns(num_ue_family.Add({{"cp_mode","spgw"},{"state","active"},{"level","pdns"}})),
current__spgw_idle_pdns(num_ue_family.Add({{"cp_mode","spgw"},{"state","idle"},{"level","pdns"}})),
current__pgw_active_pdns(num_ue_family.Add({{"cp_mode","pgw"},{"state","active"},{"level","pdns"}})),
current__pgw_idle_pdns(num_ue_family.Add({{"cp_mode","pgw"},{"state","idle"},{"level","pdns"}}))
{
}


num_ue_gauges::~num_ue_gauges()
{
}




data_usage_gauges::data_usage_gauges():
data_usage_family(BuildGauge().Name("data_usage_of_subscribers").Help("Number of Bytes transferred by UE").Labels({{"usage","data"}}).Register(*registry)),
current__pgw_pdn(data_usage_family.Add({{"cp_mode","pgw"},{"level","pdn"}})),
current__spgw_pdn(data_usage_family.Add({{"cp_mode","spgw"},{"level","pdn"}}))
{
}


data_usage_gauges::~data_usage_gauges()
{
}




subscribers_info_gauges::subscribers_info_gauges():
subscribers_info_family(BuildGauge().Name("subscribers_info").Help("subcriber connection details").Labels({{"subscriber_info","details"}}).Register(*registry)),
current__spgw_pdn(subscribers_info_family.Add({{"cp_mode","spgw"},{"level","pdn"}}))
{
}


subscribers_info_gauges::~subscribers_info_gauges()
{
}




msg_rx_counters::msg_rx_counters():
msg_rx_family(BuildCounter().Name("number_of_messages_received").Help("Number of messages received by SPGW").Labels({{"direction","incoming"}}).Register(*registry)),
msg_rx_gtpv2_s11_csreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","CSReq"}})),
msg_rx_gtpv2_s11_csreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","CSReq_drop"}})),
msg_rx_gtpv2_s5s8_csreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","CSReq"}})),
msg_rx_gtpv2_s5s8_csreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","CSReq_drop"}})),
msg_rx_gtpv2_s11_mbreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","MBReq"}})),
msg_rx_gtpv2_s11_mbreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","MBReq_drop"}})),
msg_rx_gtpv2_s5s8_mbreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","MBReq"}})),
msg_rx_gtpv2_s5s8_mbreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","MBReq_drop"}})),
msg_rx_gtpv2_s11_dsreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DSReq"}})),
msg_rx_gtpv2_s11_dsreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","DSReq_drop"}})),
msg_rx_gtpv2_s5s8_dsreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","DSReq"}})),
msg_rx_gtpv2_s5s8_dsreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","DSReq_drop"}})),
msg_rx_gtpv2_s11_rabreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","RABReq"}})),
msg_rx_gtpv2_s11_rabreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","RABReq_drop"}})),
msg_rx_gtpv2_s11_ddnack(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DDNAck"}})),
msg_rx_gtpv2_s11_ddnack_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","DDNAck_drop"}})),
msg_rx_gtpv2_s11_cbrsp(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","CBRsp"}})),
msg_rx_gtpv2_s11_cbrsp_rej(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","CBRsp_rej"}})),
msg_rx_gtpv2_s11_cbrsp_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","CBRsp_drop"}})),
msg_rx_gtpv2_s11_ubrsp(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","UBRsp"}})),
msg_rx_gtpv2_s11_ubrsp_rej(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","UBRsp_rej"}})),
msg_rx_gtpv2_s11_ubrsp_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","UBRsp_drop"}})),
msg_rx_gtpv2_s11_dbrsp(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DBRsp"}})),
msg_rx_gtpv2_s11_dbrsp_rej(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","DBRsp_rej"}})),
msg_rx_gtpv2_s11_dbrsp_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","DBRsp_drop"}})),
msg_rx_gtpv2_s11_echoreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","ECHOReq"}})),
msg_rx_gtpv2_s5s8_echoreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","ECHOReq"}})),
msg_rx_gtpv2_s11_echorsp(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","ECHORsp"}})),
msg_rx_gtpv2_s5s8_echorsp(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","ECHORsp"}})),
msg_rx_pfcp_sxa_assocsetupreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","AssocSetupReq"}})),
msg_rx_pfcp_sxa_assocsetupreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","AssocSetupReq_drop"}})),
msg_rx_pfcp_sxb_assocsetupreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","AssocSetupReq"}})),
msg_rx_pfcp_sxb_assocsetupreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","AssocSetupReq_drop"}})),
msg_rx_pfcp_sxasxb_assocsetupreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","AssocSetupReq"}})),
msg_rx_pfcp_sxasxb_assocsetupreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","AssocSetupReq_drop"}})),
msg_rx_pfcp_sxa_assocsetuprsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","AssocSetupRsp"}})),
msg_rx_pfcp_sxa_assocsetuprsq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","AssocSetupRsq_drop"}})),
msg_rx_pfcp_sxb_assocsetuprsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","AssocSetupRsp"}})),
msg_rx_pfcp_sxb_assocsetuprsq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","AssocSetupRsq_drop"}})),
msg_rx_pfcp_sxasxb_assocsetuprsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","AssocSetupRsp"}})),
msg_rx_pfcp_sxasxb_assocsetuprsq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","AssocSetupRsq_drop"}})),
msg_rx_pfcp_sxa_pfdmgmtrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","PFDMGMTRsp"}})),
msg_rx_pfcp_sxa_pfdmgmtrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","PFDMGMTRsp_drop"}})),
msg_rx_pfcp_sxb_pfdmgmtrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","PFDMGMTRsp"}})),
msg_rx_pfcp_sxb_pfdmgmtrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","PFDMGMTRsp_drop"}})),
msg_rx_pfcp_sxasxb_pfdmgmtrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","PFDMGMTRsp"}})),
msg_rx_pfcp_sxasxb_pfdmgmtrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","PFDMGMTRsp_drop"}})),
msg_rx_pfcp_sxa_echoreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","ECHOReq"}})),
msg_rx_pfcp_sxb_echoreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","ECHOReq"}})),
msg_rx_pfcp_sxasxb_echoreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","ECHOReq"}})),
msg_rx_pfcp_sxa_echorsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","ECHORsp"}})),
msg_rx_pfcp_sxb_echorsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","ECHORsp"}})),
msg_rx_pfcp_sxasxb_echorsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","ECHORsp"}})),
msg_rx_pfcp_sxa_sessestrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessEstRsp"}})),
msg_rx_pfcp_sxa_sessestrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","SessEstRsp_drop"}})),
msg_rx_pfcp_sxb_sessestrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessEstRsp"}})),
msg_rx_pfcp_sxb_sessestrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","SessEstRsp_drop"}})),
msg_rx_pfcp_sxasxb_sessestrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessEstRsp"}})),
msg_rx_pfcp_sxasxb_sessestrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","SessEstRsp_drop"}})),
msg_rx_pfcp_sxa_sessmodrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessModRsp"}})),
msg_rx_pfcp_sxa_sessmodrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","SessModRsp_drop"}})),
msg_rx_pfcp_sxb_sessmodrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessModRsp"}})),
msg_rx_pfcp_sxb_sessmodrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","SessModRsp_drop"}})),
msg_rx_pfcp_sxasxb_sessmodrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessModRsp"}})),
msg_rx_pfcp_sxasxb_sessmodrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","SessModRsp_drop"}})),
msg_rx_pfcp_sxa_sessdelrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessDelRsp"}})),
msg_rx_pfcp_sxa_sessdelrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","SessDelRsp_drop"}})),
msg_rx_pfcp_sxb_sessdelrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessDelRsp"}})),
msg_rx_pfcp_sxb_sessdelrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","SessDelRsp_drop"}})),
msg_rx_pfcp_sxasxb_sessdelrsp(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessDelRsp"}})),
msg_rx_pfcp_sxasxb_sessdelrsp_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","SessDelRsp_drop"}})),
msg_rx_pfcp_sxa_sessreportreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessReportReq"}})),
msg_rx_pfcp_sxa_sessreportreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_drop","SessReportReq_drop"}})),
msg_rx_pfcp_sxa_sessreportreq_rej(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_rej","SessReportReq_rej"}})),
msg_rx_pfcp_sxb_sessreportreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessReportReq"}})),
msg_rx_pfcp_sxb_sessreportreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_drop","SessReportReq_drop"}})),
msg_rx_pfcp_sxb_sessreportreq_rej(msg_rx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_rej","SessReportReq_rej"}})),
msg_rx_pfcp_sxasxb_sessreportreq(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessReportReq"}})),
msg_rx_pfcp_sxasxb_sessreportreq_drop(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_drop","SessReportReq_drop"}})),
msg_rx_pfcp_sxasxb_sessreportreq_rej(msg_rx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_rej","SessReportReq_rej"}})),
msg_rx_diameter_gx_cca_i(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCA_I"}})),
msg_rx_diameter_gx_cca_i_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_I_drop"}})),
msg_rx_diameter_gx_cca_u(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCA_U"}})),
msg_rx_diameter_gx_cca_u_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_U_drop"}})),
msg_rx_diameter_gx_cca_t(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCA_T"}})),
msg_rx_diameter_gx_cca_t_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_T_drop"}})),
msg_rx_diameter_gx_rar(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","RAR"}})),
msg_rx_diameter_gx_rar_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","RAR_drop"}}))
{
}


msg_rx_counters::~msg_rx_counters()
{
}




msg_tx_counters::msg_tx_counters():
msg_tx_family(BuildCounter().Name("number_of_messages_sent").Help("Number of messages sent by SPGW").Labels({{"direction","outgoing"}}).Register(*registry)),
msg_tx_gtpv2_s11_version_not_supported(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","version_not_supported"}})),
msg_tx_gtpv2_s5s8_version_not_supported(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","version_not_supported"}})),
msg_tx_gtpv2_s11_csrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","CSRsp"}})),
msg_tx_gtpv2_s11_csrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","CSRsp_rej"}})),
msg_tx_gtpv2_s5s8_csrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","CSRsp"}})),
msg_tx_gtpv2_s5s8_csrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","CSRsp_rej"}})),
msg_tx_gtpv2_s11_mbrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","MBRsp"}})),
msg_tx_gtpv2_s11_mbrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","MBRsp_rej"}})),
msg_tx_gtpv2_s5s8_mbrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","MBRsp"}})),
msg_tx_gtpv2_s5s8_mbrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","MBRsp_rej"}})),
msg_tx_gtpv2_s11_dsrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DSRsp"}})),
msg_tx_gtpv2_s11_dsrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","DSRsp_rej"}})),
msg_tx_gtpv2_s5s8_dsrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","DSRsp"}})),
msg_tx_gtpv2_s5s8_dsrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","DSRsp_rej"}})),
msg_tx_gtpv2_s11_rabrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","RABRsp"}})),
msg_tx_gtpv2_s11_rabrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","RABRsp_rej"}})),
msg_tx_gtpv2_s5s8_rabrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","RABRsp"}})),
msg_tx_gtpv2_s5s8_rabrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","RABRsp_rej"}})),
msg_tx_gtpv2_s11_ddnreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DDNReq"}})),
msg_tx_gtpv2_s11_cbreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","CBReq"}})),
msg_tx_gtpv2_s11_ubreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","UBReq"}})),
msg_tx_gtpv2_s11_dbreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DBReq"}})),
msg_tx_gtpv2_s11_echoreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","ECHOReq"}})),
msg_tx_gtpv2_s5s8_echoreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","ECHOReq"}})),
msg_tx_gtpv2_s11_echorsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","ECHORsp"}})),
msg_tx_gtpv2_s5s8_echorsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","ECHORsp"}})),
msg_tx_pfcp_sxa_assocsetupreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","AssocSetupReq"}})),
msg_tx_pfcp_sxb_assocsetupreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","AssocSetupReq"}})),
msg_tx_pfcp_sxasxb_assocsetupreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","AssocSetupReq"}})),
msg_tx_pfcp_sxa_assocsetuprsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","AssocSetupRsp"}})),
msg_tx_pfcp_sxa_assocsetuprsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_rej","AssocSetupRsp_rej"}})),
msg_tx_pfcp_sxb_assocsetuprsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","AssocSetupRsp"}})),
msg_tx_pfcp_sxb_assocsetuprsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_rej","AssocSetupRsp_rej"}})),
msg_tx_pfcp_sxasxb_assocsetuprsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","AssocSetupRsp"}})),
msg_tx_pfcp_sxasxb_assocsetuprsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_rej","AssocSetupRsp_rej"}})),
msg_tx_pfcp_sxa_pfdmgmtreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","PFDMGMTReq"}})),
msg_tx_pfcp_sxb_pfdmgmtreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","PFDMGMTReq"}})),
msg_tx_pfcp_sxasxb_pfdmgmtreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","PFDMGMTReq"}})),
msg_tx_pfcp_sxa_echoreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","ECHOReq"}})),
msg_tx_pfcp_sxb_echoreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","ECHOReq"}})),
msg_tx_pfcp_sxasxb_echoreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","ECHOReq"}})),
msg_tx_pfcp_sxa_echorsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","ECHORsp"}})),
msg_tx_pfcp_sxb_echorsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","ECHORsp"}})),
msg_tx_pfcp_sxasxb_echorsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","ECHORsp"}})),
msg_tx_pfcp_sxa_sessestreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessEstReq"}})),
msg_tx_pfcp_sxb_sessestreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessEstReq"}})),
msg_tx_pfcp_sxasxb_sessestreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessEstReq"}})),
msg_tx_pfcp_sxa_sessmodreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessModReq"}})),
msg_tx_pfcp_sxb_sessmodreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessModReq"}})),
msg_tx_pfcp_sxasxb_sessmodreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessModReq"}})),
msg_tx_pfcp_sxa_sessdelreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessDelReq"}})),
msg_tx_pfcp_sxb_sessdelreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessDelReq"}})),
msg_tx_pfcp_sxasxb_sessdelreq(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessDelReq"}})),
msg_tx_pfcp_sxa_sessreportrsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_type","SessReportRsp"}})),
msg_tx_pfcp_sxa_sessreportrsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxa"},{"msg_rej","SessReportRsp_rej"}})),
msg_tx_pfcp_sxb_sessreportrsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_type","SessReportRsp"}})),
msg_tx_pfcp_sxb_sessreportrsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","Sxb"},{"msg_rej","SessReportRsp_rej"}})),
msg_tx_pfcp_sxasxb_sessreportrsp(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_type","SessReportRsp"}})),
msg_tx_pfcp_sxasxb_sessreportrsp_rej(msg_tx_family.Add({{"protocol","pfcp"},{"interface","SxaSxb"},{"msg_rej","SessReportRsp_rej"}})),
msg_tx_diameter_gx_ccr_i(msg_tx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCR_I"}})),
msg_tx_diameter_gx_ccr_u(msg_tx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCR_U"}})),
msg_tx_diameter_gx_ccr_t(msg_tx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCR_T"}})),
msg_tx_diameter_gx_raa(msg_tx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","RAA"}})),
msg_tx_diameter_gx_raa_rej(msg_tx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_rej","RAA_rej"}}))
{
}


msg_tx_counters::~msg_tx_counters()
{
}




procedures_counters::procedures_counters():
procedures_family(BuildCounter().Name("number_of_procedures").Help("Number of procedures executed/started by spgw").Labels({{"gw","procedures"}}).Register(*registry)),
procedures_pgw_initial_attach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","INITIAL_ATTACH"}})),
procedures_spgw_initial_attach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","INITIAL_ATTACH"}})),
procedures_pgw_initial_attach_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","INITIAL_ATTACH"},{"result","success"}})),
procedures_pgw_initial_attach_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","INITIAL_ATTACH"},{"result","failure"}})),
procedures_spgw_initial_attach_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","INITIAL_ATTACH"},{"result","success"}})),
procedures_spgw_initial_attach_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","INITIAL_ATTACH"},{"result","failure"}})),
procedures_pgw_mme_init_detach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","MME_INIT_DETACH"}})),
procedures_pgw_nw_init_detach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_DETACH"}})),
procedures_spgw_mme_init_detach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","MME_INIT_DETACH"}})),
procedures_spgw_nw_init_detach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_DETACH"}})),
procedures_pgw_mme_init_detach_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","MME_INIT_DETACH"},{"result","success"}})),
procedures_pgw_mme_init_detach_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","MME_INIT_DETACH"},{"result","failure"}})),
procedures_pgw_nw_init_detach_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_DETACH"},{"result","success"}})),
procedures_pgw_nw_init_detach_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_DETACH"},{"result","failure"}})),
procedures_spgw_mme_init_detach_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","MME_INIT_DETACH"},{"result","success"}})),
procedures_spgw_mme_init_detach_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","MME_INIT_DETACH"},{"result","failure"}})),
procedures_spgw_nw_init_detach_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_DETACH"},{"result","success"}})),
procedures_spgw_nw_init_detach_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_DETACH"},{"result","failure"}})),
procedures_spgw_s1_release(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"}})),
procedures_spgw_s1_release_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"},{"result","success"}})),
procedures_spgw_s1_release_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"},{"result","failure"}})),
procedures_spgw_service_request_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"}})),
procedures_spgw_service_request_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","success"}})),
procedures_spgw_service_request_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","failure"}})),
procedures_pgw_dedicated_bearer_activation_proc(procedures_family.Add({{"cp_mode","pgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"}})),
procedures_spgw_dedicated_bearer_activation_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"}})),
procedures_pgw_dedicated_bearer_activation_proc_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"},{"result","success"}})),
procedures_pgw_dedicated_bearer_activation_proc_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"},{"result","failure"}})),
procedures_spgw_dedicated_bearer_activation_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"},{"result","success"}})),
procedures_spgw_dedicated_bearer_activation_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","DEDICATED_BEARER_ACTIVATION_PROC"},{"result","failure"}})),
procedures_pgw_bearer_update_proc(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_UPDATE_PROC"}})),
procedures_spgw_bearer_update_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_UPDATE_PROC"}})),
procedures_pgw_bearer_update_proc_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_UPDATE_PROC"},{"result","success"}})),
procedures_pgw_bearer_update_proc_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_UPDATE_PROC"},{"result","failure"}})),
procedures_spgw_bearer_update_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_UPDATE_PROC"},{"result","success"}})),
procedures_spgw_bearer_update_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_UPDATE_PROC"},{"result","failure"}})),
procedures_pgw_bearer_delete_proc(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_DELETE_PROC"}})),
procedures_spgw_bearer_delete_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_DELETE_PROC"}})),
procedures_pgw_bearer_delete_proc_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_DELETE_PROC"},{"result","success"}})),
procedures_pgw_bearer_delete_proc_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","BEARER_DELETE_PROC"},{"result","failure"}})),
procedures_spgw_bearer_delete_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_DELETE_PROC"},{"result","success"}})),
procedures_spgw_bearer_delete_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","BEARER_DELETE_PROC"},{"result","failure"}})),
procedures_pgw_nw_init_pdn_delete_proc(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"}})),
procedures_spgw_nw_init_pdn_delete_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"}})),
procedures_pgw_nw_init_pdn_delete_proc_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"},{"result","success"}})),
procedures_pgw_nw_init_pdn_delete_proc_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"},{"result","failure"}})),
procedures_spgw_nw_init_pdn_delete_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"},{"result","success"}})),
procedures_spgw_nw_init_pdn_delete_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_PDN_DELETE_PROC"},{"result","failure"}})),
procedures_pgw_rar_proc(procedures_family.Add({{"cp_mode","pgw"},{"procedure","RAR_PROC"}})),
procedures_spgw_rar_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","RAR_PROC"}})),
procedures_pgw_rar_proc_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","RAR_PROC"},{"result","success"}})),
procedures_pgw_rar_proc_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","RAR_PROC"},{"result","failure"}})),
procedures_spgw_rar_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","RAR_PROC"},{"result","success"}})),
procedures_spgw_rar_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","RAR_PROC"},{"result","failure"}}))
{
}


procedures_counters::~procedures_counters()
{
}




void spgwStats::increment(spgwStatsCounter name,std::map<std::string,std::string> labels)
{
	switch(name) {
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_active_subscribers.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_idle_subscribers.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_active_subscribers.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_idle_subscribers.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_PDNS:
	{
		num_ue_m->current__spgw_active_pdns.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_PDNS:
	{
		num_ue_m->current__spgw_idle_pdns.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_PDNS:
	{
		num_ue_m->current__pgw_active_pdns.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_PDNS:
	{
		num_ue_m->current__pgw_idle_pdns.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_PGW_PDN:
	{
		data_usage_m->current__pgw_pdn.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject1 *obj = data_usage_m->add_dynamic1("cp_mode","pgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject2 *obj = data_usage_m->add_dynamic2("cp_mode","pgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject3 *obj = data_usage_m->add_dynamic3("cp_mode","pgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_SPGW_PDN:
	{
		data_usage_m->current__spgw_pdn.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject1 *obj = data_usage_m->add_dynamic1("cp_mode","spgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject2 *obj = data_usage_m->add_dynamic2("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    data_usage_DynamicMetricObject3 *obj = data_usage_m->add_dynamic3("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::SUBSCRIBERS_INFO_SPGW_PDN:
	{
		subscribers_info_m->current__spgw_pdn.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject1 *obj = static_cast<subscribers_info_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    subscribers_info_DynamicMetricObject1 *obj = subscribers_info_m->add_dynamic1("cp_mode","spgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject2 *obj = static_cast<subscribers_info_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    subscribers_info_DynamicMetricObject2 *obj = subscribers_info_m->add_dynamic2("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject3 *obj = static_cast<subscribers_info_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Increment();
		} else {
		    subscribers_info_DynamicMetricObject3 *obj = subscribers_info_m->add_dynamic3("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_CSREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_csreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","CSReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","CSReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","CSReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_CSREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_csreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","CSReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","CSReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","CSReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_CSREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_csreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","CSReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","CSReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","CSReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_CSREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_csreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_drop","CSReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_drop","CSReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_drop","CSReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_MBREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_mbreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","MBReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","MBReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","MBReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_MBREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_mbreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","MBReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","MBReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","MBReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_MBREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_mbreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","MBReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","MBReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","MBReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_MBREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_mbreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_drop","MBReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_drop","MBReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_drop","MBReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DSREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_dsreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DSReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DSReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DSReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DSREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_dsreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","DSReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","DSReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","DSReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_DSREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_dsreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","DSReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","DSReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","DSReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_DSREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_dsreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_drop","DSReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_drop","DSReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_drop","DSReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_RABREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_rabreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","RABReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","RABReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","RABReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_RABREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_rabreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","RABReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","RABReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","RABReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DDNACK:
	{
		msg_rx_m->msg_rx_gtpv2_s11_ddnack.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DDNAck",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DDNAck",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DDNAck",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DDNACK_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_ddnack_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","DDNAck_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","DDNAck_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","DDNAck_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_CBRSP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_cbrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","CBRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","CBRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","CBRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_CBRSP_REJ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_cbrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","CBRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","CBRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","CBRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_CBRSP_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_cbrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","CBRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","CBRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","CBRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_UBRSP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_ubrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","UBRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","UBRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","UBRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_UBRSP_REJ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_ubrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","UBRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","UBRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","UBRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_UBRSP_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_ubrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","UBRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","UBRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","UBRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DBRSP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_dbrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DBRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DBRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DBRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DBRSP_REJ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_dbrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","DBRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","DBRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","DBRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_DBRSP_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_dbrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_drop","DBRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_drop","DBRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_drop","DBRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_ECHOREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s11_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_ECHOREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S11_ECHORSP:
	{
		msg_rx_m->msg_rx_gtpv2_s11_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_ECHORSP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ASSOCSETUPREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxa_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ASSOCSETUPREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_assocsetupreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ASSOCSETUPREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxb_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ASSOCSETUPREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_assocsetupreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_assocsetupreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ASSOCSETUPRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ASSOCSETUPRSQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_assocsetuprsq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupRsq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ASSOCSETUPRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ASSOCSETUPRSQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_assocsetuprsq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupRsq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ASSOCSETUPRSQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_assocsetuprsq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupRsq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupRsq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_PFDMGMTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_pfdmgmtrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_PFDMGMTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_pfdmgmtrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_PFDMGMTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_pfdmgmtrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_PFDMGMTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_pfdmgmtrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_PFDMGMTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_pfdmgmtrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_PFDMGMTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_pfdmgmtrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","PFDMGMTRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ECHOREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxa_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ECHOREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxb_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ECHOREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_ECHORSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_ECHORSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_ECHORSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSESTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessestrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessEstRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSESTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessestrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","SessEstRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSESTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessestrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessEstRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSESTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessestrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","SessEstRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSESTRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessestrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessEstRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessEstRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSESTRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessestrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","SessEstRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","SessEstRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSMODRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessmodrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessModRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSMODRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessmodrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","SessModRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSMODRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessmodrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessModRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSMODRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessmodrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","SessModRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSMODRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessmodrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessModRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessModRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSMODRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessmodrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","SessModRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","SessModRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSDELRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessdelrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessDelRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSDELRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessdelrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","SessDelRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSDELRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessdelrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessDelRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSDELRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessdelrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","SessDelRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSDELRSP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessdelrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessDelRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessDelRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSDELRSP_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessdelrsp_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","SessDelRsp_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","SessDelRsp_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSREPORTREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessreportreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessReportReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSREPORTREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessreportreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_drop","SessReportReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXA_SESSREPORTREQ_REJ:
	{
		msg_rx_m->msg_rx_pfcp_sxa_sessreportreq_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_rej","SessReportReq_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSREPORTREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessreportreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessReportReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSREPORTREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessreportreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_drop","SessReportReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXB_SESSREPORTREQ_REJ:
	{
		msg_rx_m->msg_rx_pfcp_sxb_sessreportreq_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_rej","SessReportReq_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSREPORTREQ:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessreportreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessReportReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessReportReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessreportreq_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_drop","SessReportReq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_drop","SessReportReq_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_PFCP_SXASXB_SESSREPORTREQ_REJ:
	{
		msg_rx_m->msg_rx_pfcp_sxasxb_sessreportreq_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportReq_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportReq_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_I:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_i.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCA_I",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCA_I",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCA_I",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_I_DROP:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_i_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_drop","CCA_I_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_drop","CCA_I_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_drop","CCA_I_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_U:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_u.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCA_U",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCA_U",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCA_U",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_U_DROP:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_u_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_drop","CCA_U_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_drop","CCA_U_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_drop","CCA_U_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_T:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_t.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCA_T",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCA_T",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCA_T",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_CCA_T_DROP:
	{
		msg_rx_m->msg_rx_diameter_gx_cca_t_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_drop","CCA_T_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_drop","CCA_T_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_drop","CCA_T_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_RAR:
	{
		msg_rx_m->msg_rx_diameter_gx_rar.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","RAR",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","RAR",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","RAR",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_DIAMETER_GX_RAR_DROP:
	{
		msg_rx_m->msg_rx_diameter_gx_rar_drop.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject1 *obj = static_cast<msg_rx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject1 *obj = msg_rx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_drop","RAR_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject2 *obj = static_cast<msg_rx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject2 *obj = msg_rx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_drop","RAR_drop",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_rx_DynamicMetricObject3 *obj = static_cast<msg_rx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject3 *obj = msg_rx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_drop","RAR_drop",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_VERSION_NOT_SUPPORTED:
	{
		msg_tx_m->msg_tx_gtpv2_s11_version_not_supported.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","version_not_supported",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","version_not_supported",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","version_not_supported",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_VERSION_NOT_SUPPORTED:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_version_not_supported.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","version_not_supported",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","version_not_supported",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","version_not_supported",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_CSRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s11_csrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","CSRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","CSRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","CSRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_CSRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_csrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_rej","CSRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_rej","CSRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_rej","CSRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_CSRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_csrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","CSRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","CSRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","CSRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_CSRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_csrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_rej","CSRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_rej","CSRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_rej","CSRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_MBRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s11_mbrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","MBRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","MBRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","MBRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_MBRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_mbrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_rej","MBRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_rej","MBRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_rej","MBRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_MBRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_mbrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","MBRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","MBRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","MBRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_MBRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_mbrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_rej","MBRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_rej","MBRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_rej","MBRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_DSRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s11_dsrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DSRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DSRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DSRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_DSRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_dsrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_rej","DSRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_rej","DSRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_rej","DSRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_DSRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_dsrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","DSRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","DSRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","DSRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_DSRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_dsrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_rej","DSRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_rej","DSRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_rej","DSRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_RABRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s11_rabrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","RABRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","RABRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","RABRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_RABRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_rabrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_rej","RABRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_rej","RABRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_rej","RABRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_RABRSP:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_rabrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","RABRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","RABRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","RABRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_RABRSP_REJ:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_rabrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_rej","RABRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_rej","RABRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_rej","RABRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_DDNREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_ddnreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DDNReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DDNReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DDNReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_CBREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_cbreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","CBReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","CBReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","CBReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_UBREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_ubreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","UBReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","UBReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","UBReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_DBREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_dbreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","DBReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","DBReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","DBReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_ECHOREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s11_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_ECHOREQ:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S11_ECHORSP:
	{
		msg_tx_m->msg_tx_gtpv2_s11_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_GTPV2_S5S8_ECHORSP:
	{
		msg_tx_m->msg_tx_gtpv2_s5s8_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_ASSOCSETUPREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_ASSOCSETUPREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_ASSOCSETUPREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_assocsetupreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_ASSOCSETUPRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxa_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_ASSOCSETUPRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_assocsetuprsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_rej","AssocSetupRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_ASSOCSETUPRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxb_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_ASSOCSETUPRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_assocsetuprsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_rej","AssocSetupRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_ASSOCSETUPRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_assocsetuprsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_ASSOCSETUPRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_assocsetuprsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_rej","AssocSetupRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_rej","AssocSetupRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_PFDMGMTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_pfdmgmtreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_PFDMGMTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_pfdmgmtreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_PFDMGMTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_pfdmgmtreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_ECHOREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_ECHOREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_ECHOREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_echoreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_ECHORSP:
	{
		msg_tx_m->msg_tx_pfcp_sxa_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_ECHORSP:
	{
		msg_tx_m->msg_tx_pfcp_sxb_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_ECHORSP:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_echorsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_SESSESTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_sessestreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessEstReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_SESSESTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_sessestreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessEstReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_SESSESTREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_sessestreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessEstReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessEstReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_SESSMODREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_sessmodreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessModReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_SESSMODREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_sessmodreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessModReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_SESSMODREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_sessmodreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessModReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessModReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_SESSDELREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_sessdelreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessDelReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_SESSDELREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_sessdelreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessDelReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_SESSDELREQ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_sessdelreq.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessDelReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessDelReq",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_SESSREPORTRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxa_sessreportrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_type","SessReportRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXA_SESSREPORTRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxa_sessreportrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxa","msg_rej","SessReportRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxa","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxa","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_SESSREPORTRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxb_sessreportrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_type","SessReportRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXB_SESSREPORTRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxb_sessreportrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","Sxb","msg_rej","SessReportRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","Sxb","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","Sxb","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_SESSREPORTRSP:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_sessreportrsp.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_type","SessReportRsp",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_type","SessReportRsp",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_PFCP_SXASXB_SESSREPORTRSP_REJ:
	{
		msg_tx_m->msg_tx_pfcp_sxasxb_sessreportrsp_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportRsp_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportRsp_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_DIAMETER_GX_CCR_I:
	{
		msg_tx_m->msg_tx_diameter_gx_ccr_i.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCR_I",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCR_I",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCR_I",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_DIAMETER_GX_CCR_U:
	{
		msg_tx_m->msg_tx_diameter_gx_ccr_u.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCR_U",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCR_U",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCR_U",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_DIAMETER_GX_CCR_T:
	{
		msg_tx_m->msg_tx_diameter_gx_ccr_t.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","CCR_T",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","CCR_T",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","CCR_T",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_DIAMETER_GX_RAA:
	{
		msg_tx_m->msg_tx_diameter_gx_raa.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_type","RAA",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_type","RAA",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_type","RAA",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_TX_DIAMETER_GX_RAA_REJ:
	{
		msg_tx_m->msg_tx_diameter_gx_raa_rej.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject1 *obj = static_cast<msg_tx_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject1 *obj = msg_tx_m->add_dynamic1("protocol","diameter","interface","Gx","msg_rej","RAA_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject2 *obj = static_cast<msg_tx_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject2 *obj = msg_tx_m->add_dynamic2("protocol","diameter","interface","Gx","msg_rej","RAA_rej",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    msg_tx_DynamicMetricObject3 *obj = static_cast<msg_tx_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject3 *obj = msg_tx_m->add_dynamic3("protocol","diameter","interface","Gx","msg_rej","RAA_rej",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_INITIAL_ATTACH:
	{
		procedures_m->procedures_pgw_initial_attach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","INITIAL_ATTACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","INITIAL_ATTACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","INITIAL_ATTACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_INITIAL_ATTACH:
	{
		procedures_m->procedures_spgw_initial_attach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","INITIAL_ATTACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","INITIAL_ATTACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","INITIAL_ATTACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_INITIAL_ATTACH_SUCCESS:
	{
		procedures_m->procedures_pgw_initial_attach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","INITIAL_ATTACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","INITIAL_ATTACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","INITIAL_ATTACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_INITIAL_ATTACH_FAILURE:
	{
		procedures_m->procedures_pgw_initial_attach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","INITIAL_ATTACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","INITIAL_ATTACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","INITIAL_ATTACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_INITIAL_ATTACH_SUCCESS:
	{
		procedures_m->procedures_spgw_initial_attach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","INITIAL_ATTACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","INITIAL_ATTACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","INITIAL_ATTACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_INITIAL_ATTACH_FAILURE:
	{
		procedures_m->procedures_spgw_initial_attach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","INITIAL_ATTACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","INITIAL_ATTACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","INITIAL_ATTACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_MME_INIT_DETACH:
	{
		procedures_m->procedures_pgw_mme_init_detach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","MME_INIT_DETACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","MME_INIT_DETACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","MME_INIT_DETACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_DETACH:
	{
		procedures_m->procedures_pgw_nw_init_detach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_DETACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_DETACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_DETACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_MME_INIT_DETACH:
	{
		procedures_m->procedures_spgw_mme_init_detach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","MME_INIT_DETACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","MME_INIT_DETACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","MME_INIT_DETACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_DETACH:
	{
		procedures_m->procedures_spgw_nw_init_detach.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_DETACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_DETACH",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_DETACH",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_MME_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_pgw_mme_init_detach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","MME_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","MME_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","MME_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_MME_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_pgw_mme_init_detach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","MME_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","MME_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","MME_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_pgw_nw_init_detach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_pgw_nw_init_detach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_MME_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_spgw_mme_init_detach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","MME_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","MME_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","MME_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_MME_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_spgw_mme_init_detach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","MME_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","MME_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","MME_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_spgw_nw_init_detach_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_DETACH","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_spgw_nw_init_detach_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_DETACH","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_S1_RELEASE:
	{
		procedures_m->procedures_spgw_s1_release.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","S1_RELEASE",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","S1_RELEASE",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","S1_RELEASE",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_S1_RELEASE_SUCCESS:
	{
		procedures_m->procedures_spgw_s1_release_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","S1_RELEASE","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","S1_RELEASE","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","S1_RELEASE","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_S1_RELEASE_FAILURE:
	{
		procedures_m->procedures_spgw_s1_release_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","S1_RELEASE","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","S1_RELEASE","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","S1_RELEASE","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_SERVICE_REQUEST_PROC:
	{
		procedures_m->procedures_spgw_service_request_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_SERVICE_REQUEST_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_service_request_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_SERVICE_REQUEST_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_service_request_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_DEDICATED_BEARER_ACTIVATION_PROC:
	{
		procedures_m->procedures_pgw_dedicated_bearer_activation_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_DEDICATED_BEARER_ACTIVATION_PROC:
	{
		procedures_m->procedures_spgw_dedicated_bearer_activation_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_DEDICATED_BEARER_ACTIVATION_PROC_SUCCESS:
	{
		procedures_m->procedures_pgw_dedicated_bearer_activation_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_DEDICATED_BEARER_ACTIVATION_PROC_FAILURE:
	{
		procedures_m->procedures_pgw_dedicated_bearer_activation_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_DEDICATED_BEARER_ACTIVATION_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_dedicated_bearer_activation_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_DEDICATED_BEARER_ACTIVATION_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_dedicated_bearer_activation_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","DEDICATED_BEARER_ACTIVATION_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_UPDATE_PROC:
	{
		procedures_m->procedures_pgw_bearer_update_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_UPDATE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_UPDATE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_UPDATE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_UPDATE_PROC:
	{
		procedures_m->procedures_spgw_bearer_update_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_UPDATE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_UPDATE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_UPDATE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_UPDATE_PROC_SUCCESS:
	{
		procedures_m->procedures_pgw_bearer_update_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_UPDATE_PROC_FAILURE:
	{
		procedures_m->procedures_pgw_bearer_update_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_UPDATE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_UPDATE_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_bearer_update_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_UPDATE_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_bearer_update_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_UPDATE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_DELETE_PROC:
	{
		procedures_m->procedures_pgw_bearer_delete_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_DELETE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_DELETE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_DELETE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_DELETE_PROC:
	{
		procedures_m->procedures_spgw_bearer_delete_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_DELETE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_DELETE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_DELETE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_DELETE_PROC_SUCCESS:
	{
		procedures_m->procedures_pgw_bearer_delete_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_BEARER_DELETE_PROC_FAILURE:
	{
		procedures_m->procedures_pgw_bearer_delete_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","BEARER_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_DELETE_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_bearer_delete_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_BEARER_DELETE_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_bearer_delete_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","BEARER_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_PDN_DELETE_PROC:
	{
		procedures_m->procedures_pgw_nw_init_pdn_delete_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_PDN_DELETE_PROC:
	{
		procedures_m->procedures_spgw_nw_init_pdn_delete_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_PDN_DELETE_PROC_SUCCESS:
	{
		procedures_m->procedures_pgw_nw_init_pdn_delete_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_NW_INIT_PDN_DELETE_PROC_FAILURE:
	{
		procedures_m->procedures_pgw_nw_init_pdn_delete_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_PDN_DELETE_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_nw_init_pdn_delete_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_NW_INIT_PDN_DELETE_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_nw_init_pdn_delete_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","NW_INIT_PDN_DELETE_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_RAR_PROC:
	{
		procedures_m->procedures_pgw_rar_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","RAR_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","RAR_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","RAR_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_RAR_PROC:
	{
		procedures_m->procedures_spgw_rar_proc.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","RAR_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","RAR_PROC",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","RAR_PROC",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_RAR_PROC_SUCCESS:
	{
		procedures_m->procedures_pgw_rar_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","RAR_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","RAR_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","RAR_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_PGW_RAR_PROC_FAILURE:
	{
		procedures_m->procedures_pgw_rar_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","pgw","procedure","RAR_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","pgw","procedure","RAR_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","pgw","procedure","RAR_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_RAR_PROC_SUCCESS:
	{
		procedures_m->procedures_spgw_rar_proc_success.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","RAR_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","RAR_PROC","result","success",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","RAR_PROC","result","success",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SPGW_RAR_PROC_FAILURE:
	{
		procedures_m->procedures_spgw_rar_proc_failure.Increment();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject1 *obj = static_cast<procedures_DynamicMetricObject1 *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject1 *obj = procedures_m->add_dynamic1("cp_mode","spgw","procedure","RAR_PROC","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject2 *obj = static_cast<procedures_DynamicMetricObject2 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject2 *obj = procedures_m->add_dynamic2("cp_mode","spgw","procedure","RAR_PROC","result","failure",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    procedures_DynamicMetricObject3 *obj = static_cast<procedures_DynamicMetricObject3 *>(itf->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject3 *obj = procedures_m->add_dynamic3("cp_mode","spgw","procedure","RAR_PROC","result","failure",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	default:
		break;
	}
}




void spgwStats::decrement(spgwStatsCounter name,std::map<std::string,std::string> labels)
{
	switch(name) {
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_active_subscribers.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_idle_subscribers.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_active_subscribers.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_idle_subscribers.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_PDNS:
	{
		num_ue_m->current__spgw_active_pdns.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_PDNS:
	{
		num_ue_m->current__spgw_idle_pdns.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_PDNS:
	{
		num_ue_m->current__pgw_active_pdns.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_PDNS:
	{
		num_ue_m->current__pgw_idle_pdns.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_PGW_PDN:
	{
		data_usage_m->current__pgw_pdn.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_SPGW_PDN:
	{
		data_usage_m->current__spgw_pdn.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	case spgwStatsCounter::SUBSCRIBERS_INFO_SPGW_PDN:
	{
		subscribers_info_m->current__spgw_pdn.Decrement();
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject1 *obj = static_cast<subscribers_info_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(it1);
		      delete obj;
		     }
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it1->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject2 *obj = static_cast<subscribers_info_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		     }
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject3 *obj = static_cast<subscribers_info_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Decrement();
		    if(obj->gauge.Value() == 0) {
		      metrics_map.erase(itf);
		      delete obj;
		    }
		}
		}
		break;
	}
	default:
		break;
	}
}




void spgwStats::set(spgwStatsCounter name, double val, std::map<std::string,std::string> labels)
{
	switch(name) {
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_active_subscribers.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_idle_subscribers.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_active_subscribers.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","active","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_idle_subscribers.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","idle","level","subscribers",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_PDNS:
	{
		num_ue_m->current__spgw_active_pdns.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_PDNS:
	{
		num_ue_m->current__spgw_idle_pdns.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","spgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","spgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","spgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_PDNS:
	{
		num_ue_m->current__pgw_active_pdns.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","active","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_PDNS:
	{
		num_ue_m->current__pgw_idle_pdns.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject1 *obj = static_cast<num_ue_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject1 *obj = num_ue_m->add_dynamic1("cp_mode","pgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject2 *obj = static_cast<num_ue_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject2 *obj = num_ue_m->add_dynamic2("cp_mode","pgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    num_ue_DynamicMetricObject3 *obj = static_cast<num_ue_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    num_ue_DynamicMetricObject3 *obj = num_ue_m->add_dynamic3("cp_mode","pgw","state","idle","level","pdns",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_PGW_PDN:
	{
		data_usage_m->current__pgw_pdn.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject1 *obj = data_usage_m->add_dynamic1("cp_mode","pgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject2 *obj = data_usage_m->add_dynamic2("cp_mode","pgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject3 *obj = data_usage_m->add_dynamic3("cp_mode","pgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::DATA_USAGE_SPGW_PDN:
	{
		data_usage_m->current__spgw_pdn.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    data_usage_DynamicMetricObject1 *obj = static_cast<data_usage_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject1 *obj = data_usage_m->add_dynamic1("cp_mode","spgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject2 *obj = static_cast<data_usage_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject2 *obj = data_usage_m->add_dynamic2("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    data_usage_DynamicMetricObject3 *obj = static_cast<data_usage_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    data_usage_DynamicMetricObject3 *obj = data_usage_m->add_dynamic3("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	case spgwStatsCounter::SUBSCRIBERS_INFO_SPGW_PDN:
	{
		subscribers_info_m->current__spgw_pdn.Set(val);
		if(labels.size() == 0) {
		break;
		}
		if(labels.size() == 1) {
		auto it = labels. begin();
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject1 *obj = static_cast<subscribers_info_DynamicMetricObject1 *>(it1->second);
		    obj->gauge.Set(val);
		} else {
		    subscribers_info_DynamicMetricObject1 *obj = subscribers_info_m->add_dynamic1("cp_mode","spgw","level","pdn",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		} else if (labels.size() == 2) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		struct Node s1 = {name, it1->first+it2->first, it2->second+it2->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject2 *obj = static_cast<subscribers_info_DynamicMetricObject2 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    subscribers_info_DynamicMetricObject2 *obj = subscribers_info_m->add_dynamic2("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		} 
		} else if (labels.size() == 3) {
		auto it1 = labels. begin();
		auto it2 = it1++;
		auto it3 = it2++;
		struct Node s1 = {name, it1->first+it2->first+it3->first, it1->second+it2->second+it3->second};
		auto itf = metrics_map.find(s1);
		if(itf != metrics_map.end()) {
		    subscribers_info_DynamicMetricObject3 *obj = static_cast<subscribers_info_DynamicMetricObject3 *>(itf->second);
		    obj->gauge.Set(val);
		} else {
		    subscribers_info_DynamicMetricObject3 *obj = subscribers_info_m->add_dynamic3("cp_mode","spgw","level","pdn",it1->first, it1->second, it2->first, it2->second, it3->first, it3->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Set(val);
		}
		}
		break;
	}
	default:
		break;
	}
}


#ifdef TEST_PROMETHEUS 
#include <unistd.h>
int main() {
	std::thread prom(spgwStatsSetupPrometheusThread, 3081);
	prom.detach();
	sleep(1);
	while(1) {
	spgwStats::Instance()->increment(spgwStatsCounter::NUM_UE_SPGW_ACTIVE_SUBSCRIBERS, {{"mme_addr","1.1.1.1"},{"spgwu_addr", "1.1.1.2"}});
	sleep(1);
	}
}
#endif
