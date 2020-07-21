// Copyright 2020-present Open Networking Foundation
//
// SPDX-License-Identifier: LicenseRef-ONF-Member-Only-1.0

#include <iostream>
#include <map>
#include "spgw_tables.h"

bool 
spgwTables::add_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans)
{
    transKey_t key = {msg_seq, src_addr, src_port};
    std::pair <transKey_t,void*> foo;
    foo = std::make_pair(key, trans);
    spgw_pfcp_transaction_map.insert(foo);
    return true;
}

void* 
spgwTables::find_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq)
{
    std::map<transKey_t, void*>::iterator it;
    transKey_t key = {msg_seq, src_addr, src_port};
    it = spgw_pfcp_transaction_map.find(key);
    if(it == spgw_pfcp_transaction_map.end())
    {
#ifdef DEBUG_LOG_CPP
        std::cout<<"Key not Found"<<std::endl;
#endif
        return NULL;
    }
    else
    {
#if DEBUG_LOG_CPP
        std::cout<<"Key Found - it->second "<<it->second<<std::endl;
#endif
        void *temp = it->second;
        return temp;
    }
}

void* 
spgwTables::delete_pfcp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq)
{
    std::map<transKey_t, void*>::iterator it;
    transKey_t key = {msg_seq, src_addr, src_port};
    it = spgw_pfcp_transaction_map.find(key);
    if(it == spgw_pfcp_transaction_map.end())
    {
#ifdef DEBUG_LOG_CPP
        std::cout<<"Key not Found"<<std::endl;
#endif
        return NULL;
    }
    else
    {
#ifdef DEBUG_LOG_CPP
        std::cout<<"Key Found - it->second "<<it->second<<std::endl;
#endif
        void *temp = it->second;
        spgw_pfcp_transaction_map.erase(it);
        return temp;
    }
}

bool 
spgwTables::add_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq, void *trans)
{
    transKey_t key = {msg_seq, src_addr, src_port};
    std::pair <transKey_t,void*> foo;
    foo = std::make_pair(key, trans);
    spgw_gtp_transaction_map.insert(foo);
    return true;
}

void* 
spgwTables::find_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq)
{
    std::map<transKey_t, void*>::iterator it;
    transKey_t key = {msg_seq, src_addr, src_port};
    it = spgw_gtp_transaction_map.find(key);
    if(it == spgw_gtp_transaction_map.end())
    {
#if DEBUG_LOG_CPP
        std::cout<<"Key not Found"<<std::endl;
#endif
        return NULL;
    }
    else
    {
#if DEBUG_LOG_CPP
        std::cout<<"Key Found second "<<it->second<<std::endl;
#endif
        void *temp = it->second;
        return temp;
    }
}

void* 
spgwTables::delete_gtp_trans(uint32_t src_addr, uint16_t src_port, uint32_t msg_seq)
{
    std::map<transKey_t, void*>::iterator it;
    transKey_t key = {msg_seq, src_addr, src_port};
    it = spgw_gtp_transaction_map.find(key);
    if(it == spgw_gtp_transaction_map.end())
    {
#if DEBUG_LOG_CPP
        std::cout<<"Key not Found"<<std::endl;
#endif
        return NULL;
    }
    else
    {
#if DEBUG_LOG_CPP
        std::cout<<"Key Found - it->second "<<it->second<<std::endl;
#endif
        void *temp = it->second;
        spgw_gtp_transaction_map.erase(it);
        return temp;
    }
}

void spgwTables::queue_event(void *context)
{
    stack_events_queue.push(context);
    return;
}

void* spgwTables::pop_event(void)
{
    if(stack_events_queue.empty())
       return NULL;
    void *context = stack_events_queue.front();
    stack_events_queue.pop();
    return context;
}