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

DEFINE_VAPI_MSG_IDS_L2_API_JSON;

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
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.bd_id = m_hw_item.data();
    payload.flood = 1;
    payload.uu_flood = 1;
    payload.forward = 1;
    payload.learn = 1;
    payload.arp_term= 1;
    payload.mac_age = 1;
    payload.is_add = 1;

    VAPI_CALL(req.execute());

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
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.bd_id = m_hw_item.data();
    payload.is_add = 0;

    VAPI_CALL(req.execute());

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
    m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

    auto &payload = m_dump->get_request().get_payload();
    payload.bd_id = ~0;

    VAPI_CALL(m_dump->execute());

    wait();

    return rc_t::OK;
}

std::string BridgeDomain::DumpCmd::to_string() const
{
    return ("bridge-domain-dump");
}
