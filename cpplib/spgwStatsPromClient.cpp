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
#include "spgwStatsPromClient.h"

using namespace prometheus;
std::shared_ptr<Registry> registry;

void spgwStatsSetupPrometheusThread(void)
{
    registry = std::make_shared<Registry>();
    /* Create single instance */ 
    spgwStats::Instance(); 
    Exposer exposer{"0.0.0.0:3081", 1};
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
num_ue_family(BuildGauge().Name("mme_number_of_ue_attached").Help("Number of UE attached").Labels({{"spgw_num_ue","subscribers"}}).Register(*registry)),
current__spgw_active_subscribers(num_ue_family.Add({{"cp_mode","spgw"},{"state","active"},{"level","subscribers"}})),
current__spgw_idle_subscribers(num_ue_family.Add({{"cp_mode","spgw"},{"state","idle"},{"level","subscribers"}})),
current__pgw_active_subscribers(num_ue_family.Add({{"cp_mode","pgw"},{"state","active"},{"level","subscribers"}})),
current__pgw_idle_subscribers(num_ue_family.Add({{"cp_mode","pgw"},{"state","idle"},{"level","subscribers"}})),
current__sgw_active_subscribers(num_ue_family.Add({{"cp_mode","sgw"},{"state","active"},{"level","subscribers"}})),
current__sgw_idle_subscribers(num_ue_family.Add({{"cp_mode","sgw"},{"state","idle"},{"level","subscribers"}})),
current__spgw_active_pdns(num_ue_family.Add({{"cp_mode","spgw"},{"state","active"},{"level","pdns"}})),
current__spgw_idle_pdns(num_ue_family.Add({{"cp_mode","spgw"},{"state","idle"},{"level","pdns"}})),
current__pgw_active_pdns(num_ue_family.Add({{"cp_mode","pgw"},{"state","active"},{"level","pdns"}})),
current__pgw_idle_pdns(num_ue_family.Add({{"cp_mode","pgw"},{"state","idle"},{"level","pdns"}})),
current__sgw_active_pdns(num_ue_family.Add({{"cp_mode","sgw"},{"state","active"},{"level","pdns"}})),
current__sgw_idle_pdns(num_ue_family.Add({{"cp_mode","sgw"},{"state","idle"},{"level","pdns"}}))
{
}


num_ue_gauges::~num_ue_gauges()
{
}




msg_rx_counters::msg_rx_counters():
msg_rx_family(BuildCounter().Name("number_of_messages_received").Help("Number of messages received by SPGW").Labels({{"direction","incoming"}}).Register(*registry)),
msg_rx_gtpv2_s11_csreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","CSReq"}})),
msg_rx_gtpv2_s11_csreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","CSReq_drop"}})),
msg_rx_gtpv2_s5s8_csreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","CSReq"}})),
msg_rx_gtpv2_s5s8_csreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","CSReq_drop"}})),
msg_rx_gtpv2_s11_mbreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","mbreq"}})),
msg_rx_gtpv2_s11_mbreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","mbreq_drop"}})),
msg_rx_gtpv2_s5s8_mbreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","mbreq"}})),
msg_rx_gtpv2_s5s8_mbreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","mbreq_drop"}})),
msg_rx_gtpv2_s11_dsreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DSReq"}})),
msg_rx_gtpv2_s11_dsreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","dsreq_drop"}})),
msg_rx_gtpv2_s5s8_dsreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","DSReq"}})),
msg_rx_gtpv2_s5s8_dsreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","dsreq_drop"}})),
msg_rx_gtpv2_s11_rabreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","RABReq"}})),
msg_rx_gtpv2_s11_rabreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","rabreq_drop"}})),
msg_rx_gtpv2_s5s8_rabreq(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","RABReq"}})),
msg_rx_gtpv2_s5s8_rabreq_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","rabreq_drop"}})),
msg_rx_gtpv2_s11_ddnack(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DDNAck"}})),
msg_rx_gtpv2_s11_ddnack_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_drop","ddnack_drop"}})),
msg_rx_gtpv2_s5s8_ddnack(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","DDNAck"}})),
msg_rx_gtpv2_s5s8_ddnack_drop(msg_rx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_drop","ddnack_drop"}})),
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
msg_rx_diameter_gx_cca_i_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_I_DROP"}})),
msg_rx_diameter_gx_cca_u(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCA_U"}})),
msg_rx_diameter_gx_cca_u_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_U_DROP"}})),
msg_rx_diameter_gx_cca_t(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","CCA_T"}})),
msg_rx_diameter_gx_cca_t_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","CCA_T_DROP"}})),
msg_rx_diameter_gx_rar(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_type","RAR"}})),
msg_rx_diameter_gx_rar_drop(msg_rx_family.Add({{"protocol","diameter"},{"interface","Gx"},{"msg_drop","RAR_DROP"}}))
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
msg_tx_gtpv2_s11_csrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","CSRsp_REJ"}})),
msg_tx_gtpv2_s5s8_csrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","CSRsp"}})),
msg_tx_gtpv2_s5s8_csrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","CSRsp_REJ"}})),
msg_tx_gtpv2_s11_mbrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","MBRsp"}})),
msg_tx_gtpv2_s11_mbrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","MBRsp_REJ"}})),
msg_tx_gtpv2_s5s8_mbrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","MBRsp"}})),
msg_tx_gtpv2_s5s8_mbrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","MBRsp_REJ"}})),
msg_tx_gtpv2_s11_dsrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DSRsp"}})),
msg_tx_gtpv2_s11_dsrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","DSRsp_rej"}})),
msg_tx_gtpv2_s5s8_dsrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","DSRsp"}})),
msg_tx_gtpv2_s5s8_dsrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","DSRsp_rej"}})),
msg_tx_gtpv2_s11_rabrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","RABRsp"}})),
msg_tx_gtpv2_s11_rabrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_rej","RABRSP_rej"}})),
msg_tx_gtpv2_s5s8_rabrsp(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_type","RABRsp"}})),
msg_tx_gtpv2_s5s8_rabrsp_rej(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s5s8"},{"msg_rej","RABRSP_rej"}})),
msg_tx_gtpv2_s11_ddnreq(msg_tx_family.Add({{"protocol","gtpv2"},{"interface","s11"},{"msg_type","DDNReq"}})),
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
procedures_sgw_initial_attach(procedures_family.Add({{"cp_mode","sgw"},{"procedure","INITIAL_ATTACH"}})),
procedures_pgw_initial_attach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","INITIAL_ATTACH"}})),
procedures_spgw_initial_attach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","INITIAL_ATTACH"}})),
procedures_sgw_initial_attach_success(procedures_family.Add({{"cp_mode","sgw"},{"proc_type","INITIAL_ATTACH"},{"result","success"}})),
procedures_sgw_initial_attach_failure(procedures_family.Add({{"cp_mode","sgw"},{"proc_type","INITIAL_ATTACH"},{"result","failure"}})),
procedures_pgw_initial_attach_success(procedures_family.Add({{"cp_mode","pgw"},{"proc_type","INITIAL_ATTACH"},{"result","success"}})),
procedures_pgw_initial_attach_failure(procedures_family.Add({{"cp_mode","pgw"},{"proc_type","INITIAL_ATTACH"},{"result","failure"}})),
procedures_spgw_initial_attach_success(procedures_family.Add({{"cp_mode","spgw"},{"proc_type","INITIAL_ATTACH"},{"result","success"}})),
procedures_spgw_initial_attach_failure(procedures_family.Add({{"cp_mode","spgw"},{"proc_type","INITIAL_ATTACH"},{"result","failure"}})),
procedures_sgw_mme_init_detach(procedures_family.Add({{"cp_mode","sgw"},{"procedure","mme_init_detach"}})),
procedures_sgw_nw_init_detach(procedures_family.Add({{"cp_mode","sgw"},{"procedure","nw_init_detach"}})),
procedures_pgw_mme_init_detach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","mme_init_detach"}})),
procedures_pgw_nw_init_detach(procedures_family.Add({{"cp_mode","pgw"},{"procedure","nw_init_detach"}})),
procedures_spgw_mme_init_detach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","mme_init_detach"}})),
procedures_spgw_nw_init_detach(procedures_family.Add({{"cp_mode","spgw"},{"procedure","nw_init_detach"}})),
procedures_sgw_mme_init_detach_success(procedures_family.Add({{"cp_mode","sgw"},{"procedure","MME_INIT_DETACH"},{"result","success"}})),
procedures_sgw_mme_init_detach_failure(procedures_family.Add({{"cp_mode","sgw"},{"procedure","MME_INIT_DETACH"},{"result","failure"}})),
procedures_sgw_nw_init_detach_success(procedures_family.Add({{"cp_mode","sgw"},{"procedure","NW_INIT_DETACH"},{"result","success"}})),
procedures_sgw_nw_init_detach_failure(procedures_family.Add({{"cp_mode","sgw"},{"procedure","NW_INIT_DETACH"},{"result","failure"}})),
procedures_pgw_mme_init_detach_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","MME_INIT_DETACH"},{"result","success"}})),
procedures_pgw_mme_init_detach_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","MME_INIT_DETACH"},{"result","failure"}})),
procedures_pgw_nw_init_detach_success(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_DETACH"},{"result","success"}})),
procedures_pgw_nw_init_detach_failure(procedures_family.Add({{"cp_mode","pgw"},{"procedure","NW_INIT_DETACH"},{"result","failure"}})),
procedures_spgw_mme_init_detach_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","MME_INIT_DETACH"},{"result","success"}})),
procedures_spgw_mme_init_detach_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","MME_INIT_DETACH"},{"result","failure"}})),
procedures_spgw_nw_init_detach_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_DETACH"},{"result","success"}})),
procedures_spgw_nw_init_detach_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","NW_INIT_DETACH"},{"result","failure"}})),
procedures_sgw_s1_release(procedures_family.Add({{"cp_mode","sgw"},{"procedure","S1_RELEASE"}})),
procedures_spgw_s1_release(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"}})),
procedures_sgw_s1_release_success(procedures_family.Add({{"cp_mode","sgw"},{"procedure","S1_RELEASE"},{"result","success"}})),
procedures_sgw_s1_release_failure(procedures_family.Add({{"cp_mode","sgw"},{"procedure","S1_RELEASE"},{"result","failure"}})),
procedures_spgw_s1_release_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"},{"result","success"}})),
procedures_spgw_s1_release_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","S1_RELEASE"},{"result","failure"}})),
procedures_sgw_service_request_proc(procedures_family.Add({{"cp_mode","sgw"},{"procedure","SERVICE_REQUEST_PROC"}})),
procedures_spgw_service_request_proc(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"}})),
procedures_sgw_service_request_proc_success(procedures_family.Add({{"cp_mode","sgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","success"}})),
procedures_sgw_service_request_proc_failure(procedures_family.Add({{"cp_mode","sgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","failure"}})),
procedures_spgw_service_request_proc_success(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","success"}})),
procedures_spgw_service_request_proc_failure(procedures_family.Add({{"cp_mode","spgw"},{"procedure","SERVICE_REQUEST_PROC"},{"result","failure"}}))
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","active","level","subscribers",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","idle","level","subscribers",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","active","level","subscribers",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__sgw_active_subscribers.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__sgw_idle_subscribers.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","idle","level","subscribers",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","active","level","pdns",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","idle","level","pdns",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","active","level","pdns",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_ACTIVE_PDNS:
	{
		num_ue_m->current__sgw_active_pdns.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_IDLE_PDNS:
	{
		num_ue_m->current__sgw_idle_pdns.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Increment();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","idle","level","pdns",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","CSReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_drop","CSReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","CSReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_drop","CSReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","mbreq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_drop","mbreq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","mbreq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_drop","mbreq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","DSReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_drop","dsreq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","DSReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_drop","dsreq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","RABReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_drop","rabreq_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_RABREQ:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_rabreq.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","RABReq",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_RABREQ_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_rabreq_drop.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_drop","rabreq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","DDNAck",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_drop","ddnack_drop",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_DDNACK:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_ddnack.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","DDNAck",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::MSG_RX_GTPV2_S5S8_DDNACK_DROP:
	{
		msg_rx_m->msg_rx_gtpv2_s5s8_ddnack_drop.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_drop","ddnack_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","AssocSetupRsq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","AssocSetupRsq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","AssocSetupRsq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","PFDMGMTRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessEstRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","SessEstRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessEstRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","SessEstRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessEstRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","SessEstRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessModRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","SessModRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessModRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","SessModRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessModRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","SessModRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessDelRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","SessDelRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessDelRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","SessDelRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessDelRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","SessDelRsp_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessReportReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_drop","SessReportReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_rej","SessReportReq_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessReportReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_drop","SessReportReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_rej","SessReportReq_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessReportReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_drop","SessReportReq_drop",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportReq_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCA_I",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_drop","CCA_I_DROP",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCA_U",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_drop","CCA_U_DROP",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCA_T",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_drop","CCA_T_DROP",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","RAR",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_rx_DynamicMetricObject *obj = static_cast<msg_rx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_rx_DynamicMetricObject *obj = msg_rx_m->add_dynamic("protocol","diameter","interface","Gx","msg_drop","RAR_DROP",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","version_not_supported",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","version_not_supported",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","CSRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_rej","CSRsp_REJ",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","CSRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_rej","CSRsp_REJ",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","MBRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_rej","MBRsp_REJ",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","MBRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_rej","MBRsp_REJ",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","DSRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_rej","DSRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","DSRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_rej","DSRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","RABRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_rej","RABRSP_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","RABRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_rej","RABRSP_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","DDNReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s11","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","gtpv2","interface","s5s8","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_rej","AssocSetupRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_rej","AssocSetupRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","AssocSetupRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_rej","AssocSetupRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","PFDMGMTReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","PFDMGMTReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","PFDMGMTReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","ECHOReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","ECHORsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessEstReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessEstReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessEstReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessModReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessModReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessModReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessDelReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessDelReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessDelReq",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_type","SessReportRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxa","msg_rej","SessReportRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_type","SessReportRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","Sxb","msg_rej","SessReportRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_type","SessReportRsp",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","pfcp","interface","SxaSxb","msg_rej","SessReportRsp_rej",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCR_I",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCR_U",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","CCR_T",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","diameter","interface","Gx","msg_type","RAA",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    msg_tx_DynamicMetricObject *obj = static_cast<msg_tx_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    msg_tx_DynamicMetricObject *obj = msg_tx_m->add_dynamic("protocol","diameter","interface","Gx","msg_rej","RAA_rej",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_INITIAL_ATTACH:
	{
		procedures_m->procedures_sgw_initial_attach.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","INITIAL_ATTACH",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","INITIAL_ATTACH",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","INITIAL_ATTACH",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_INITIAL_ATTACH_SUCCESS:
	{
		procedures_m->procedures_sgw_initial_attach_success.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","proc_type","INITIAL_ATTACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_INITIAL_ATTACH_FAILURE:
	{
		procedures_m->procedures_sgw_initial_attach_failure.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","proc_type","INITIAL_ATTACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","proc_type","INITIAL_ATTACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","proc_type","INITIAL_ATTACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","proc_type","INITIAL_ATTACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","proc_type","INITIAL_ATTACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_MME_INIT_DETACH:
	{
		procedures_m->procedures_sgw_mme_init_detach.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","mme_init_detach",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_NW_INIT_DETACH:
	{
		procedures_m->procedures_sgw_nw_init_detach.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","nw_init_detach",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","mme_init_detach",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","nw_init_detach",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","mme_init_detach",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","nw_init_detach",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_MME_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_sgw_mme_init_detach_success.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","MME_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_MME_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_sgw_mme_init_detach_failure.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","MME_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_NW_INIT_DETACH_SUCCESS:
	{
		procedures_m->procedures_sgw_nw_init_detach_success.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","NW_INIT_DETACH","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_NW_INIT_DETACH_FAILURE:
	{
		procedures_m->procedures_sgw_nw_init_detach_failure.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","NW_INIT_DETACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","MME_INIT_DETACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","MME_INIT_DETACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","NW_INIT_DETACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","pgw","procedure","NW_INIT_DETACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","MME_INIT_DETACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","MME_INIT_DETACH","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","NW_INIT_DETACH","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","NW_INIT_DETACH","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_S1_RELEASE:
	{
		procedures_m->procedures_sgw_s1_release.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","S1_RELEASE",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","S1_RELEASE",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_S1_RELEASE_SUCCESS:
	{
		procedures_m->procedures_sgw_s1_release_success.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","S1_RELEASE","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_S1_RELEASE_FAILURE:
	{
		procedures_m->procedures_sgw_s1_release_failure.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","S1_RELEASE","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","S1_RELEASE","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","S1_RELEASE","result","failure",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_SERVICE_REQUEST_PROC:
	{
		procedures_m->procedures_sgw_service_request_proc.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","SERVICE_REQUEST_PROC",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_SERVICE_REQUEST_PROC_SUCCESS:
	{
		procedures_m->procedures_sgw_service_request_proc_success.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","SERVICE_REQUEST_PROC","result","success",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->counter.Increment();
		}
		}
		break;
	}
	case spgwStatsCounter::PROCEDURES_SGW_SERVICE_REQUEST_PROC_FAILURE:
	{
		procedures_m->procedures_sgw_service_request_proc_failure.Increment();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","sgw","procedure","SERVICE_REQUEST_PROC","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","success",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    procedures_DynamicMetricObject *obj = static_cast<procedures_DynamicMetricObject *>(it1->second);
		    obj->counter.Increment();
		} else {
		    procedures_DynamicMetricObject *obj = procedures_m->add_dynamic("cp_mode","spgw","procedure","SERVICE_REQUEST_PROC","result","failure",it->first, it->second);
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
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__spgw_idle_subscribers.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_active_subscribers.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__pgw_idle_subscribers.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_ACTIVE_SUBSCRIBERS:
	{
		num_ue_m->current__sgw_active_subscribers.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","active","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_IDLE_SUBSCRIBERS:
	{
		num_ue_m->current__sgw_idle_subscribers.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","idle","level","subscribers",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_ACTIVE_PDNS:
	{
		num_ue_m->current__spgw_active_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SPGW_IDLE_PDNS:
	{
		num_ue_m->current__spgw_idle_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","spgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_ACTIVE_PDNS:
	{
		num_ue_m->current__pgw_active_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_PGW_IDLE_PDNS:
	{
		num_ue_m->current__pgw_idle_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","pgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_ACTIVE_PDNS:
	{
		num_ue_m->current__sgw_active_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","active","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
		}
		}
		break;
	}
	case spgwStatsCounter::NUM_UE_SGW_IDLE_PDNS:
	{
		num_ue_m->current__sgw_idle_pdns.Decrement();
		for(auto it = labels.begin(); it != labels.end(); it++) {
		std::cout<<"label - ("<<it->first<<","<<it->second<<")"<<std::endl;
		struct Node s1 = {name, it->first, it->second};
		auto it1 = metrics_map.find(s1);
		if(it1 != metrics_map.end()) {
		    num_ue_DynamicMetricObject *obj = static_cast<num_ue_DynamicMetricObject *>(it1->second);
		    obj->gauge.Decrement();
		} else {
		    num_ue_DynamicMetricObject *obj = num_ue_m->add_dynamic("cp_mode","sgw","state","idle","level","pdns",it->first, it->second);
		    auto p1 = std::make_pair(s1, obj);
		    metrics_map.insert(p1);
		    obj->gauge.Decrement();
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
	std::thread prom(spgwStatsSetupPrometheusThread);
	prom.detach();
	while(1) {
	spgwStats::Instance()->increment(spgwStatsCounter::NUM_UE_SPGW_ACTIVE_SUBSCRIBERS);
	sleep(1);
	}
}
#endif