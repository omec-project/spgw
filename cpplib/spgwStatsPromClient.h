/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _INCLUDE_spgwStats_H__
#define _INCLUDE_spgwStats_H__

#include <prometheus/gauge.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>


using namespace prometheus;
extern std::shared_ptr<Registry> registry;


void spgwStatsSetupPrometheusThread(void);

enum class spgwStatsCounter {
	NUM_UE_SPGW_ACTIVE_SUBSCRIBERS,
	NUM_UE_SPGW_IDLE_SUBSCRIBERS,
	NUM_UE_PGW_ACTIVE_SUBSCRIBERS,
	NUM_UE_PGW_IDLE_SUBSCRIBERS,
	NUM_UE_SGW_ACTIVE_SUBSCRIBERS,
	NUM_UE_SGW_IDLE_SUBSCRIBERS,
	NUM_UE_SPGW_ACTIVE_PDNS,
	NUM_UE_SPGW_IDLE_PDNS,
	NUM_UE_PGW_ACTIVE_PDNS,
	NUM_UE_PGW_IDLE_PDNS,
	NUM_UE_SGW_ACTIVE_PDNS,
	NUM_UE_SGW_IDLE_PDNS,
	MSG_RX_GTPV2_S11_CSREQ,
	MSG_RX_GTPV2_S11_CSREQ_DROP,
	MSG_RX_GTPV2_S5S8_CSREQ,
	MSG_RX_GTPV2_S5S8_CSREQ_DROP,
	MSG_RX_GTPV2_S11_MBREQ,
	MSG_RX_GTPV2_S11_MBREQ_DROP,
	MSG_RX_GTPV2_S5S8_MBREQ,
	MSG_RX_GTPV2_S5S8_MBREQ_DROP,
	MSG_RX_GTPV2_S11_DSREQ,
	MSG_RX_GTPV2_S11_DSREQ_DROP,
	MSG_RX_GTPV2_S5S8_DSREQ,
	MSG_RX_GTPV2_S5S8_DSREQ_DROP,
	MSG_RX_GTPV2_S11_RABREQ,
	MSG_RX_GTPV2_S11_RABREQ_DROP,
	MSG_RX_GTPV2_S5S8_RABREQ,
	MSG_RX_GTPV2_S5S8_RABREQ_DROP,
	MSG_RX_GTPV2_S11_DDNACK,
	MSG_RX_GTPV2_S11_DDNACK_DROP,
	MSG_RX_GTPV2_S5S8_DDNACK,
	MSG_RX_GTPV2_S5S8_DDNACK_DROP,
	MSG_RX_GTPV2_S11_ECHOREQ,
	MSG_RX_GTPV2_S5S8_ECHOREQ,
	MSG_RX_GTPV2_S11_ECHORSP,
	MSG_RX_GTPV2_S5S8_ECHORSP,
	MSG_RX_PFCP_SXA_ASSOCSETUPREQ,
	MSG_RX_PFCP_SXA_ASSOCSETUPREQ_DROP,
	MSG_RX_PFCP_SXB_ASSOCSETUPREQ,
	MSG_RX_PFCP_SXB_ASSOCSETUPREQ_DROP,
	MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ,
	MSG_RX_PFCP_SXASXB_ASSOCSETUPREQ_DROP,
	MSG_RX_PFCP_SXA_ASSOCSETUPRSP,
	MSG_RX_PFCP_SXA_ASSOCSETUPRSQ_DROP,
	MSG_RX_PFCP_SXB_ASSOCSETUPRSP,
	MSG_RX_PFCP_SXB_ASSOCSETUPRSQ_DROP,
	MSG_RX_PFCP_SXASXB_ASSOCSETUPRSP,
	MSG_RX_PFCP_SXASXB_ASSOCSETUPRSQ_DROP,
	MSG_RX_PFCP_SXA_PFDMGMTRSP,
	MSG_RX_PFCP_SXA_PFDMGMTRSP_DROP,
	MSG_RX_PFCP_SXB_PFDMGMTRSP,
	MSG_RX_PFCP_SXB_PFDMGMTRSP_DROP,
	MSG_RX_PFCP_SXASXB_PFDMGMTRSP,
	MSG_RX_PFCP_SXASXB_PFDMGMTRSP_DROP,
	MSG_RX_PFCP_SXA_ECHOREQ,
	MSG_RX_PFCP_SXB_ECHOREQ,
	MSG_RX_PFCP_SXASXB_ECHOREQ,
	MSG_RX_PFCP_SXA_ECHORSP,
	MSG_RX_PFCP_SXB_ECHORSP,
	MSG_RX_PFCP_SXASXB_ECHORSP,
	MSG_RX_PFCP_SXA_SESSESTRSP,
	MSG_RX_PFCP_SXA_SESSESTRSP_DROP,
	MSG_RX_PFCP_SXB_SESSESTRSP,
	MSG_RX_PFCP_SXB_SESSESTRSP_DROP,
	MSG_RX_PFCP_SXASXB_SESSESTRSP,
	MSG_RX_PFCP_SXASXB_SESSESTRSP_DROP,
	MSG_RX_PFCP_SXA_SESSMODRSP,
	MSG_RX_PFCP_SXA_SESSMODRSP_DROP,
	MSG_RX_PFCP_SXB_SESSMODRSP,
	MSG_RX_PFCP_SXB_SESSMODRSP_DROP,
	MSG_RX_PFCP_SXASXB_SESSMODRSP,
	MSG_RX_PFCP_SXASXB_SESSMODRSP_DROP,
	MSG_RX_PFCP_SXA_SESSDELRSP,
	MSG_RX_PFCP_SXA_SESSDELRSP_DROP,
	MSG_RX_PFCP_SXB_SESSDELRSP,
	MSG_RX_PFCP_SXB_SESSDELRSP_DROP,
	MSG_RX_PFCP_SXASXB_SESSDELRSP,
	MSG_RX_PFCP_SXASXB_SESSDELRSP_DROP,
	MSG_RX_PFCP_SXA_SESSREPORTREQ,
	MSG_RX_PFCP_SXA_SESSREPORTREQ_DROP,
	MSG_RX_PFCP_SXA_SESSREPORTREQ_REJ,
	MSG_RX_PFCP_SXB_SESSREPORTREQ,
	MSG_RX_PFCP_SXB_SESSREPORTREQ_DROP,
	MSG_RX_PFCP_SXB_SESSREPORTREQ_REJ,
	MSG_RX_PFCP_SXASXB_SESSREPORTREQ,
	MSG_RX_PFCP_SXASXB_SESSREPORTREQ_DROP,
	MSG_RX_PFCP_SXASXB_SESSREPORTREQ_REJ,
	MSG_RX_DIAMETER_GX_CCA_I,
	MSG_RX_DIAMETER_GX_CCA_I_DROP,
	MSG_RX_DIAMETER_GX_CCA_U,
	MSG_RX_DIAMETER_GX_CCA_U_DROP,
	MSG_RX_DIAMETER_GX_CCA_T,
	MSG_RX_DIAMETER_GX_CCA_T_DROP,
	MSG_RX_DIAMETER_GX_RAR,
	MSG_RX_DIAMETER_GX_RAR_DROP,
	MSG_TX_GTPV2_S11_VERSION_NOT_SUPPORTED,
	MSG_TX_GTPV2_S5S8_VERSION_NOT_SUPPORTED,
	MSG_TX_GTPV2_S11_CSRSP,
	MSG_TX_GTPV2_S11_CSRSP_REJ,
	MSG_TX_GTPV2_S5S8_CSRSP,
	MSG_TX_GTPV2_S5S8_CSRSP_REJ,
	MSG_TX_GTPV2_S11_MBRSP,
	MSG_TX_GTPV2_S11_MBRSP_REJ,
	MSG_TX_GTPV2_S5S8_MBRSP,
	MSG_TX_GTPV2_S5S8_MBRSP_REJ,
	MSG_TX_GTPV2_S11_DSRSP,
	MSG_TX_GTPV2_S11_DSRSP_REJ,
	MSG_TX_GTPV2_S5S8_DSRSP,
	MSG_TX_GTPV2_S5S8_DSRSP_REJ,
	MSG_TX_GTPV2_S11_RABRSP,
	MSG_TX_GTPV2_S11_RABRSP_REJ,
	MSG_TX_GTPV2_S5S8_RABRSP,
	MSG_TX_GTPV2_S5S8_RABRSP_REJ,
	MSG_TX_GTPV2_S11_DDNREQ,
	MSG_TX_GTPV2_S11_ECHOREQ,
	MSG_TX_GTPV2_S5S8_ECHOREQ,
	MSG_TX_GTPV2_S11_ECHORSP,
	MSG_TX_GTPV2_S5S8_ECHORSP,
	MSG_TX_PFCP_SXA_ASSOCSETUPREQ,
	MSG_TX_PFCP_SXB_ASSOCSETUPREQ,
	MSG_TX_PFCP_SXASXB_ASSOCSETUPREQ,
	MSG_TX_PFCP_SXA_ASSOCSETUPRSP,
	MSG_TX_PFCP_SXA_ASSOCSETUPRSP_REJ,
	MSG_TX_PFCP_SXB_ASSOCSETUPRSP,
	MSG_TX_PFCP_SXB_ASSOCSETUPRSP_REJ,
	MSG_TX_PFCP_SXASXB_ASSOCSETUPRSP,
	MSG_TX_PFCP_SXASXB_ASSOCSETUPRSP_REJ,
	MSG_TX_PFCP_SXA_PFDMGMTREQ,
	MSG_TX_PFCP_SXB_PFDMGMTREQ,
	MSG_TX_PFCP_SXASXB_PFDMGMTREQ,
	MSG_TX_PFCP_SXA_ECHOREQ,
	MSG_TX_PFCP_SXB_ECHOREQ,
	MSG_TX_PFCP_SXASXB_ECHOREQ,
	MSG_TX_PFCP_SXA_ECHORSP,
	MSG_TX_PFCP_SXB_ECHORSP,
	MSG_TX_PFCP_SXASXB_ECHORSP,
	MSG_TX_PFCP_SXA_SESSESTREQ,
	MSG_TX_PFCP_SXB_SESSESTREQ,
	MSG_TX_PFCP_SXASXB_SESSESTREQ,
	MSG_TX_PFCP_SXA_SESSMODREQ,
	MSG_TX_PFCP_SXB_SESSMODREQ,
	MSG_TX_PFCP_SXASXB_SESSMODREQ,
	MSG_TX_PFCP_SXA_SESSDELREQ,
	MSG_TX_PFCP_SXB_SESSDELREQ,
	MSG_TX_PFCP_SXASXB_SESSDELREQ,
	MSG_TX_PFCP_SXA_SESSREPORTRSP,
	MSG_TX_PFCP_SXA_SESSREPORTRSP_REJ,
	MSG_TX_PFCP_SXB_SESSREPORTRSP,
	MSG_TX_PFCP_SXB_SESSREPORTRSP_REJ,
	MSG_TX_PFCP_SXASXB_SESSREPORTRSP,
	MSG_TX_PFCP_SXASXB_SESSREPORTRSP_REJ,
	MSG_TX_DIAMETER_GX_CCR_I,
	MSG_TX_DIAMETER_GX_CCR_U,
	MSG_TX_DIAMETER_GX_CCR_T,
	MSG_TX_DIAMETER_GX_RAA,
	MSG_TX_DIAMETER_GX_RAA_REJ,
	PROCEDURES_SGW_INITIAL_ATTACH,
	PROCEDURES_PGW_INITIAL_ATTACH,
	PROCEDURES_SPGW_INITIAL_ATTACH,
	PROCEDURES_SGW_INITIAL_ATTACH_SUCCESS,
	PROCEDURES_SGW_INITIAL_ATTACH_FAILURE,
	PROCEDURES_PGW_INITIAL_ATTACH_SUCCESS,
	PROCEDURES_PGW_INITIAL_ATTACH_FAILURE,
	PROCEDURES_SPGW_INITIAL_ATTACH_SUCCESS,
	PROCEDURES_SPGW_INITIAL_ATTACH_FAILURE,
	PROCEDURES_SGW_MME_INIT_DETACH,
	PROCEDURES_SGW_NW_INIT_DETACH,
	PROCEDURES_PGW_MME_INIT_DETACH,
	PROCEDURES_PGW_NW_INIT_DETACH,
	PROCEDURES_SPGW_MME_INIT_DETACH,
	PROCEDURES_SPGW_NW_INIT_DETACH,
	PROCEDURES_SGW_MME_INIT_DETACH_SUCCESS,
	PROCEDURES_SGW_MME_INIT_DETACH_FAILURE,
	PROCEDURES_SGW_NW_INIT_DETACH_SUCCESS,
	PROCEDURES_SGW_NW_INIT_DETACH_FAILURE,
	PROCEDURES_PGW_MME_INIT_DETACH_SUCCESS,
	PROCEDURES_PGW_MME_INIT_DETACH_FAILURE,
	PROCEDURES_PGW_NW_INIT_DETACH_SUCCESS,
	PROCEDURES_PGW_NW_INIT_DETACH_FAILURE,
	PROCEDURES_SPGW_MME_INIT_DETACH_SUCCESS,
	PROCEDURES_SPGW_MME_INIT_DETACH_FAILURE,
	PROCEDURES_SPGW_NW_INIT_DETACH_SUCCESS,
	PROCEDURES_SPGW_NW_INIT_DETACH_FAILURE,
	PROCEDURES_SGW_S1_RELEASE,
	PROCEDURES_SPGW_S1_RELEASE,
	PROCEDURES_SGW_S1_RELEASE_SUCCESS,
	PROCEDURES_SGW_S1_RELEASE_FAILURE,
	PROCEDURES_SPGW_S1_RELEASE_SUCCESS,
	PROCEDURES_SPGW_S1_RELEASE_FAILURE,
	PROCEDURES_SGW_SERVICE_REQUEST_PROC,
	PROCEDURES_SPGW_SERVICE_REQUEST_PROC,
	PROCEDURES_SGW_SERVICE_REQUEST_PROC_SUCCESS,
	PROCEDURES_SGW_SERVICE_REQUEST_PROC_FAILURE,
	PROCEDURES_SPGW_SERVICE_REQUEST_PROC_SUCCESS,
	PROCEDURES_SPGW_SERVICE_REQUEST_PROC_FAILURE
};

struct Node 
{
   spgwStatsCounter id;
	std::string label_k;
	std::string label_v;

	Node(spgwStatsCounter id, std::string label_k, std::string label_v)
	{
		this->id = id;
		this->label_k = label_k;
		this->label_v = label_v;
	}

	// operator== is required to compare keys in case of hash collision
	bool operator==(const Node &p) const
	{
		return label_k == p.label_k && label_v == p.label_v && id == p.id;
	}
};

struct hash_fn
{
	std::size_t operator() (const Node &node) const
	{
		std::size_t h1 = std::hash<std::string>()(node.label_k);
		std::size_t h2 = std::hash<std::string>()(node.label_v);
		std::size_t h3 = std::size_t(node.id);
		return h1 ^ h2 ^ h3;
	}
};


class DynamicMetricObject {
	public:
};


class num_ue_DynamicMetricObject : public DynamicMetricObject {
	public:
		num_ue_DynamicMetricObject(Family<Gauge> &num_ue_family,std::string labelk0, std::string labelv0,std::string labelk1, std::string labelv1,std::string labelk2, std::string labelv2,std::string labelk, std::string labelv):
		 DynamicMetricObject(),
		 gauge(num_ue_family.Add({{labelk0, labelv0},{labelk1, labelv1},{labelk2, labelv2},{labelk, labelv}}))
		{
		}
		~num_ue_DynamicMetricObject()
		{
		}
		Gauge &gauge;
};
class num_ue_gauges {
	public:
	num_ue_gauges();
	~num_ue_gauges();
	Family<Gauge> &num_ue_family;
	Gauge &current__spgw_active_subscribers;
	Gauge &current__spgw_idle_subscribers;
	Gauge &current__pgw_active_subscribers;
	Gauge &current__pgw_idle_subscribers;
	Gauge &current__sgw_active_subscribers;
	Gauge &current__sgw_idle_subscribers;
	Gauge &current__spgw_active_pdns;
	Gauge &current__spgw_idle_pdns;
	Gauge &current__pgw_active_pdns;
	Gauge &current__pgw_idle_pdns;
	Gauge &current__sgw_active_pdns;
	Gauge &current__sgw_idle_pdns;

	num_ue_DynamicMetricObject* add_dynamic(std::string labelk0, std::string labelv0,std::string labelk1, std::string labelv1,std::string labelk2, std::string labelv2,std::string labelk, std::string labelv) {
		return new num_ue_DynamicMetricObject(num_ue_family,labelk0, labelv0,labelk1, labelv1,labelk2, labelv2,labelk, labelv);
 	}
};




class msg_rx_DynamicMetricObject : public DynamicMetricObject {
	public:
		msg_rx_DynamicMetricObject(Family<Counter> &msg_rx_family,std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv):
		 DynamicMetricObject(),
		 counter(msg_rx_family.Add({{labelk0, labelv0},{labelk1, labelv1},{labelk2, labelv2},{labelk, labelv}}))
		{
		}
		~msg_rx_DynamicMetricObject()
		{
		}
		Counter &counter;
};
class msg_rx_counters {
	public:
	msg_rx_counters();
	~msg_rx_counters();
	Family<Counter> &msg_rx_family;
	Counter &msg_rx_gtpv2_s11_csreq;
	Counter &msg_rx_gtpv2_s11_csreq_drop;
	Counter &msg_rx_gtpv2_s5s8_csreq;
	Counter &msg_rx_gtpv2_s5s8_csreq_drop;
	Counter &msg_rx_gtpv2_s11_mbreq;
	Counter &msg_rx_gtpv2_s11_mbreq_drop;
	Counter &msg_rx_gtpv2_s5s8_mbreq;
	Counter &msg_rx_gtpv2_s5s8_mbreq_drop;
	Counter &msg_rx_gtpv2_s11_dsreq;
	Counter &msg_rx_gtpv2_s11_dsreq_drop;
	Counter &msg_rx_gtpv2_s5s8_dsreq;
	Counter &msg_rx_gtpv2_s5s8_dsreq_drop;
	Counter &msg_rx_gtpv2_s11_rabreq;
	Counter &msg_rx_gtpv2_s11_rabreq_drop;
	Counter &msg_rx_gtpv2_s5s8_rabreq;
	Counter &msg_rx_gtpv2_s5s8_rabreq_drop;
	Counter &msg_rx_gtpv2_s11_ddnack;
	Counter &msg_rx_gtpv2_s11_ddnack_drop;
	Counter &msg_rx_gtpv2_s5s8_ddnack;
	Counter &msg_rx_gtpv2_s5s8_ddnack_drop;
	Counter &msg_rx_gtpv2_s11_echoreq;
	Counter &msg_rx_gtpv2_s5s8_echoreq;
	Counter &msg_rx_gtpv2_s11_echorsp;
	Counter &msg_rx_gtpv2_s5s8_echorsp;
	Counter &msg_rx_pfcp_sxa_assocsetupreq;
	Counter &msg_rx_pfcp_sxa_assocsetupreq_drop;
	Counter &msg_rx_pfcp_sxb_assocsetupreq;
	Counter &msg_rx_pfcp_sxb_assocsetupreq_drop;
	Counter &msg_rx_pfcp_sxasxb_assocsetupreq;
	Counter &msg_rx_pfcp_sxasxb_assocsetupreq_drop;
	Counter &msg_rx_pfcp_sxa_assocsetuprsp;
	Counter &msg_rx_pfcp_sxa_assocsetuprsq_drop;
	Counter &msg_rx_pfcp_sxb_assocsetuprsp;
	Counter &msg_rx_pfcp_sxb_assocsetuprsq_drop;
	Counter &msg_rx_pfcp_sxasxb_assocsetuprsp;
	Counter &msg_rx_pfcp_sxasxb_assocsetuprsq_drop;
	Counter &msg_rx_pfcp_sxa_pfdmgmtrsp;
	Counter &msg_rx_pfcp_sxa_pfdmgmtrsp_drop;
	Counter &msg_rx_pfcp_sxb_pfdmgmtrsp;
	Counter &msg_rx_pfcp_sxb_pfdmgmtrsp_drop;
	Counter &msg_rx_pfcp_sxasxb_pfdmgmtrsp;
	Counter &msg_rx_pfcp_sxasxb_pfdmgmtrsp_drop;
	Counter &msg_rx_pfcp_sxa_echoreq;
	Counter &msg_rx_pfcp_sxb_echoreq;
	Counter &msg_rx_pfcp_sxasxb_echoreq;
	Counter &msg_rx_pfcp_sxa_echorsp;
	Counter &msg_rx_pfcp_sxb_echorsp;
	Counter &msg_rx_pfcp_sxasxb_echorsp;
	Counter &msg_rx_pfcp_sxa_sessestrsp;
	Counter &msg_rx_pfcp_sxa_sessestrsp_drop;
	Counter &msg_rx_pfcp_sxb_sessestrsp;
	Counter &msg_rx_pfcp_sxb_sessestrsp_drop;
	Counter &msg_rx_pfcp_sxasxb_sessestrsp;
	Counter &msg_rx_pfcp_sxasxb_sessestrsp_drop;
	Counter &msg_rx_pfcp_sxa_sessmodrsp;
	Counter &msg_rx_pfcp_sxa_sessmodrsp_drop;
	Counter &msg_rx_pfcp_sxb_sessmodrsp;
	Counter &msg_rx_pfcp_sxb_sessmodrsp_drop;
	Counter &msg_rx_pfcp_sxasxb_sessmodrsp;
	Counter &msg_rx_pfcp_sxasxb_sessmodrsp_drop;
	Counter &msg_rx_pfcp_sxa_sessdelrsp;
	Counter &msg_rx_pfcp_sxa_sessdelrsp_drop;
	Counter &msg_rx_pfcp_sxb_sessdelrsp;
	Counter &msg_rx_pfcp_sxb_sessdelrsp_drop;
	Counter &msg_rx_pfcp_sxasxb_sessdelrsp;
	Counter &msg_rx_pfcp_sxasxb_sessdelrsp_drop;
	Counter &msg_rx_pfcp_sxa_sessreportreq;
	Counter &msg_rx_pfcp_sxa_sessreportreq_drop;
	Counter &msg_rx_pfcp_sxa_sessreportreq_rej;
	Counter &msg_rx_pfcp_sxb_sessreportreq;
	Counter &msg_rx_pfcp_sxb_sessreportreq_drop;
	Counter &msg_rx_pfcp_sxb_sessreportreq_rej;
	Counter &msg_rx_pfcp_sxasxb_sessreportreq;
	Counter &msg_rx_pfcp_sxasxb_sessreportreq_drop;
	Counter &msg_rx_pfcp_sxasxb_sessreportreq_rej;
	Counter &msg_rx_diameter_gx_cca_i;
	Counter &msg_rx_diameter_gx_cca_i_drop;
	Counter &msg_rx_diameter_gx_cca_u;
	Counter &msg_rx_diameter_gx_cca_u_drop;
	Counter &msg_rx_diameter_gx_cca_t;
	Counter &msg_rx_diameter_gx_cca_t_drop;
	Counter &msg_rx_diameter_gx_rar;
	Counter &msg_rx_diameter_gx_rar_drop;

	msg_rx_DynamicMetricObject* add_dynamic(std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv) {
		return new msg_rx_DynamicMetricObject(msg_rx_family,labelk0,labelv0,labelk1,labelv1,labelk2,labelv2,labelk, labelv);
 	}
};



class msg_tx_DynamicMetricObject : public DynamicMetricObject {
	public:
		msg_tx_DynamicMetricObject(Family<Counter> &msg_tx_family,std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv):
		 DynamicMetricObject(),
		 counter(msg_tx_family.Add({{labelk0, labelv0},{labelk1, labelv1},{labelk2, labelv2},{labelk, labelv}}))
		{
		}
		~msg_tx_DynamicMetricObject()
		{
		}
		Counter &counter;
};
class msg_tx_counters {
	public:
	msg_tx_counters();
	~msg_tx_counters();
	Family<Counter> &msg_tx_family;
	Counter &msg_tx_gtpv2_s11_version_not_supported;
	Counter &msg_tx_gtpv2_s5s8_version_not_supported;
	Counter &msg_tx_gtpv2_s11_csrsp;
	Counter &msg_tx_gtpv2_s11_csrsp_rej;
	Counter &msg_tx_gtpv2_s5s8_csrsp;
	Counter &msg_tx_gtpv2_s5s8_csrsp_rej;
	Counter &msg_tx_gtpv2_s11_mbrsp;
	Counter &msg_tx_gtpv2_s11_mbrsp_rej;
	Counter &msg_tx_gtpv2_s5s8_mbrsp;
	Counter &msg_tx_gtpv2_s5s8_mbrsp_rej;
	Counter &msg_tx_gtpv2_s11_dsrsp;
	Counter &msg_tx_gtpv2_s11_dsrsp_rej;
	Counter &msg_tx_gtpv2_s5s8_dsrsp;
	Counter &msg_tx_gtpv2_s5s8_dsrsp_rej;
	Counter &msg_tx_gtpv2_s11_rabrsp;
	Counter &msg_tx_gtpv2_s11_rabrsp_rej;
	Counter &msg_tx_gtpv2_s5s8_rabrsp;
	Counter &msg_tx_gtpv2_s5s8_rabrsp_rej;
	Counter &msg_tx_gtpv2_s11_ddnreq;
	Counter &msg_tx_gtpv2_s11_echoreq;
	Counter &msg_tx_gtpv2_s5s8_echoreq;
	Counter &msg_tx_gtpv2_s11_echorsp;
	Counter &msg_tx_gtpv2_s5s8_echorsp;
	Counter &msg_tx_pfcp_sxa_assocsetupreq;
	Counter &msg_tx_pfcp_sxb_assocsetupreq;
	Counter &msg_tx_pfcp_sxasxb_assocsetupreq;
	Counter &msg_tx_pfcp_sxa_assocsetuprsp;
	Counter &msg_tx_pfcp_sxa_assocsetuprsp_rej;
	Counter &msg_tx_pfcp_sxb_assocsetuprsp;
	Counter &msg_tx_pfcp_sxb_assocsetuprsp_rej;
	Counter &msg_tx_pfcp_sxasxb_assocsetuprsp;
	Counter &msg_tx_pfcp_sxasxb_assocsetuprsp_rej;
	Counter &msg_tx_pfcp_sxa_pfdmgmtreq;
	Counter &msg_tx_pfcp_sxb_pfdmgmtreq;
	Counter &msg_tx_pfcp_sxasxb_pfdmgmtreq;
	Counter &msg_tx_pfcp_sxa_echoreq;
	Counter &msg_tx_pfcp_sxb_echoreq;
	Counter &msg_tx_pfcp_sxasxb_echoreq;
	Counter &msg_tx_pfcp_sxa_echorsp;
	Counter &msg_tx_pfcp_sxb_echorsp;
	Counter &msg_tx_pfcp_sxasxb_echorsp;
	Counter &msg_tx_pfcp_sxa_sessestreq;
	Counter &msg_tx_pfcp_sxb_sessestreq;
	Counter &msg_tx_pfcp_sxasxb_sessestreq;
	Counter &msg_tx_pfcp_sxa_sessmodreq;
	Counter &msg_tx_pfcp_sxb_sessmodreq;
	Counter &msg_tx_pfcp_sxasxb_sessmodreq;
	Counter &msg_tx_pfcp_sxa_sessdelreq;
	Counter &msg_tx_pfcp_sxb_sessdelreq;
	Counter &msg_tx_pfcp_sxasxb_sessdelreq;
	Counter &msg_tx_pfcp_sxa_sessreportrsp;
	Counter &msg_tx_pfcp_sxa_sessreportrsp_rej;
	Counter &msg_tx_pfcp_sxb_sessreportrsp;
	Counter &msg_tx_pfcp_sxb_sessreportrsp_rej;
	Counter &msg_tx_pfcp_sxasxb_sessreportrsp;
	Counter &msg_tx_pfcp_sxasxb_sessreportrsp_rej;
	Counter &msg_tx_diameter_gx_ccr_i;
	Counter &msg_tx_diameter_gx_ccr_u;
	Counter &msg_tx_diameter_gx_ccr_t;
	Counter &msg_tx_diameter_gx_raa;
	Counter &msg_tx_diameter_gx_raa_rej;

	msg_tx_DynamicMetricObject* add_dynamic(std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv) {
		return new msg_tx_DynamicMetricObject(msg_tx_family,labelk0,labelv0,labelk1,labelv1,labelk2,labelv2,labelk, labelv);
 	}
};



class procedures_DynamicMetricObject : public DynamicMetricObject {
	public:
		procedures_DynamicMetricObject(Family<Counter> &procedures_family,std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk, std::string labelv):
		 DynamicMetricObject(),
		 counter(procedures_family.Add({{labelk0, labelv0},{labelk1, labelv1},{labelk, labelv}}))
		{
		}
		procedures_DynamicMetricObject(Family<Counter> &procedures_family,std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv):
		 DynamicMetricObject(),
		 counter(procedures_family.Add({{labelk0, labelv0},{labelk1, labelv1},{labelk2, labelv2},{labelk, labelv}}))
		{
		}
		~procedures_DynamicMetricObject()
		{
		}
		Counter &counter;
};
class procedures_counters {
	public:
	procedures_counters();
	~procedures_counters();
	Family<Counter> &procedures_family;
	Counter &procedures_sgw_initial_attach;
	Counter &procedures_pgw_initial_attach;
	Counter &procedures_spgw_initial_attach;
	Counter &procedures_sgw_initial_attach_success;
	Counter &procedures_sgw_initial_attach_failure;
	Counter &procedures_pgw_initial_attach_success;
	Counter &procedures_pgw_initial_attach_failure;
	Counter &procedures_spgw_initial_attach_success;
	Counter &procedures_spgw_initial_attach_failure;
	Counter &procedures_sgw_mme_init_detach;
	Counter &procedures_sgw_nw_init_detach;
	Counter &procedures_pgw_mme_init_detach;
	Counter &procedures_pgw_nw_init_detach;
	Counter &procedures_spgw_mme_init_detach;
	Counter &procedures_spgw_nw_init_detach;
	Counter &procedures_sgw_mme_init_detach_success;
	Counter &procedures_sgw_mme_init_detach_failure;
	Counter &procedures_sgw_nw_init_detach_success;
	Counter &procedures_sgw_nw_init_detach_failure;
	Counter &procedures_pgw_mme_init_detach_success;
	Counter &procedures_pgw_mme_init_detach_failure;
	Counter &procedures_pgw_nw_init_detach_success;
	Counter &procedures_pgw_nw_init_detach_failure;
	Counter &procedures_spgw_mme_init_detach_success;
	Counter &procedures_spgw_mme_init_detach_failure;
	Counter &procedures_spgw_nw_init_detach_success;
	Counter &procedures_spgw_nw_init_detach_failure;
	Counter &procedures_sgw_s1_release;
	Counter &procedures_spgw_s1_release;
	Counter &procedures_sgw_s1_release_success;
	Counter &procedures_sgw_s1_release_failure;
	Counter &procedures_spgw_s1_release_success;
	Counter &procedures_spgw_s1_release_failure;
	Counter &procedures_sgw_service_request_proc;
	Counter &procedures_spgw_service_request_proc;
	Counter &procedures_sgw_service_request_proc_success;
	Counter &procedures_sgw_service_request_proc_failure;
	Counter &procedures_spgw_service_request_proc_success;
	Counter &procedures_spgw_service_request_proc_failure;

	procedures_DynamicMetricObject* add_dynamic(std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk, std::string labelv) {
		return new procedures_DynamicMetricObject(procedures_family,labelk0,labelv0,labelk1,labelv1,labelk, labelv);
 	}

	procedures_DynamicMetricObject* add_dynamic(std::string labelk0,std::string labelv0,std::string labelk1,std::string labelv1,std::string labelk2,std::string labelv2,std::string labelk, std::string labelv) {
		return new procedures_DynamicMetricObject(procedures_family,labelk0,labelv0,labelk1,labelv1,labelk2,labelv2,labelk, labelv);
 	}
};

class spgwStats {
	 public:
		spgwStats();
		~spgwStats() {}
		static spgwStats* Instance(); 
		void spgwStatspromThreadSetup(void);
		void increment(spgwStatsCounter name, std::map<std::string, std::string> labels={}); 
		void decrement(spgwStatsCounter name, std::map<std::string, std::string> labels={}); 
	 public:
		num_ue_gauges *num_ue_m;
		msg_rx_counters *msg_rx_m;
		msg_tx_counters *msg_tx_m;
		procedures_counters *procedures_m;
		std::unordered_map<struct Node, DynamicMetricObject*, hash_fn> metrics_map;
};

#endif /* _INCLUDE_spgwStats_H__ */
