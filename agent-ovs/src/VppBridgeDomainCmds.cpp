/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "VppBridgeDomain.hpp"
#include "VppCmd.hpp"

extern "C"
{
    #include "l2.api.vapi.h"
}

using namespace VPP;

BridgeDomain::CreateCmd::CreateCmd(HW::Item<uint32_t> &item):
    RpcCmd(item)
{
}

bool BridgeDomain::CreateCmd::operator==(const CreateCmd& other) const
{
    return (m_hw_item.data() == other.m_hw_item.data());
}

rc_t BridgeDomain::CreateCmd::issue(Connection &con)
{
    vapi_msg_bridge_domain_add_del* req;

    req = vapi_alloc_bridge_domain_add_del(con.ctx());
    req->payload.bd_id = m_hw_item.data();
    req->payload.flood = 1;
    req->payload.uu_flood = 1;
    req->payload.forward = 1;
    req->payload.learn = 1;
    req->payload.arp_term= 1;
    req->payload.mac_age = 1;
    req->payload.is_add = 1;

    VAPI_CALL(vapi_bridge_domain_add_del(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_bridge_domain_add_del_reply,
                                   CreateCmd>,
                  this));

    m_hw_item.set(wait());
                                            
    return (rc_t::OK);
}

std::string BridgeDomain::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "bridge-domain-create: " << m_hw_item.to_string();

    return (s.str());
}

BridgeDomain::DeleteCmd::DeleteCmd(HW::Item<uint32_t> &item):
    RpcCmd(item)
{
}

bool BridgeDomain::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

rc_t BridgeDomain::DeleteCmd::issue(Connection &con)
{
    vapi_msg_bridge_domain_add_del* req;

    req = vapi_alloc_bridge_domain_add_del(con.ctx());
    req->payload.bd_id = m_hw_item.data();
    req->payload.is_add = 0;

    VAPI_CALL(vapi_bridge_domain_add_del(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_bridge_domain_add_del_reply,
                  CreateCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);
                                            
    return (rc_t::OK);
}

std::string BridgeDomain::DeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "bridge-domain-delete: " << m_hw_item.to_string();

    return (s.str());
}

BridgeDomain::DumpCmd::DumpCmd()
{
}

bool BridgeDomain::DumpCmd::operator==(const DumpCmd& other) const
{
    return (true);
}

rc_t BridgeDomain::DumpCmd::issue(Connection &con)
{
    vapi_msg_bridge_domain_dump *req;

    req = vapi_alloc_bridge_domain_dump(con.ctx());
    req->payload.bd_id = ~0;

    VAPI_CALL(vapi_bridge_domain_dump(
                  con.ctx(), req,
                  DumpCmd::callback_vl<DumpCmd>,
                  DumpCmd::mk_cb_ctx(this, vapi_calc_bridge_domain_details_msg_size)));

    wait();

    return rc_t::OK;
}

std::string BridgeDomain::DumpCmd::to_string() const
{
    return ("bridge-domain-dump");
}
