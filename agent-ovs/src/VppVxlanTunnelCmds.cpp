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

#include "VppVxlanTunnel.hpp"

DEFINE_VAPI_MSG_IDS_VXLAN_API_JSON;

using namespace VPP;

VxlanTunnel::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                  const std::string &name,
                                  const endpoint_t &ep):
    Interface::CreateCmd<vapi::Vxlan_add_del_tunnel>(item, name),
    m_ep(ep)
{
}

bool VxlanTunnel::CreateCmd::operator==(const CreateCmd& other) const
{
    return (m_ep == other.m_ep &&
            Interface::CreateCmd<vapi::Vxlan_add_del_tunnel>::operator==(other));
}

rc_t VxlanTunnel::CreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.is_add = 1;
    payload.is_ipv6 = 0;
    to_bytes(m_ep.src, &payload.is_ipv6, payload.src_address);
    to_bytes(m_ep.dst, &payload.is_ipv6, payload.dst_address);
    payload.mcast_sw_if_index = ~0;
    payload.encap_vrf_id = 0;
    payload.decap_next_index = ~0;
    payload.vni = m_ep.vni;

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item)
    {
        Interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}


std::string VxlanTunnel::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "vxlan-tunnel-create: " << m_hw_item.to_string()
      << " ep:" << m_ep.to_string();

    return (s.str());
}

VxlanTunnel::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item,
                                  const endpoint_t &ep):
    Interface::DeleteCmd<vapi::Vxlan_add_del_tunnel>(item),
    m_ep(ep)
{
}

bool VxlanTunnel::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_ep == other.m_ep);
}

rc_t VxlanTunnel::DeleteCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto payload = req.get_request().get_payload();
    payload.is_add = 0;
    payload.is_ipv6 = 0;
    to_bytes(m_ep.src, &payload.is_ipv6, payload.src_address);
    to_bytes(m_ep.dst, &payload.is_ipv6, payload.dst_address);
    payload.mcast_sw_if_index = ~0;
    payload.encap_vrf_id = 0;
    payload.decap_next_index = ~0;
    payload.vni = m_ep.vni;

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    Interface::remove(m_hw_item);
    return (rc_t::OK);
}

std::string VxlanTunnel::DeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "vxlan-tunnel-delete: " << m_hw_item.to_string()
      << " ep:" << m_ep.to_string();

    return (s.str());
}

VxlanTunnel::DumpCmd::DumpCmd()
{
}

bool VxlanTunnel::DumpCmd::operator==(const DumpCmd& other) const
{
    return (true);
}

rc_t VxlanTunnel::DumpCmd::issue(Connection &con)
{
    m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

    auto &payload = m_dump->get_request().get_payload();
    payload.sw_if_index = ~0;

    VAPI_CALL(m_dump->execute());

    wait();

    return rc_t::OK;
}

std::string VxlanTunnel::DumpCmd::to_string() const
{
    return ("Vpp-VxlanTunnels-Dump");
}
