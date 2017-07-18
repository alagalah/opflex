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
    Interface::CreateCmd(item, name),
    m_ep(ep)
{
}

bool VxlanTunnel::CreateCmd::operator==(const CreateCmd& other) const
{
    return (m_ep == other.m_ep);
}

rc_t VxlanTunnel::CreateCmd::issue(Connection &con)
{
    vapi_msg_vxlan_add_del_tunnel *req;

    req = vapi_alloc_vxlan_add_del_tunnel(con.ctx());
    req->payload.is_add = 1;
    req->payload.is_ipv6 = 0;
    to_bytes(m_ep.src, &req->payload.is_ipv6, req->payload.src_address);
    to_bytes(m_ep.dst, &req->payload.is_ipv6, req->payload.dst_address);
    req->payload.mcast_sw_if_index = ~0;
    req->payload.encap_vrf_id = 0;
    req->payload.decap_next_index = ~0;
    req->payload.vni = m_ep.vni;

    VAPI_CALL(vapi_vxlan_add_del_tunnel(
                  con.ctx(),
                  req,
                  Interface::create_callback<
                      vapi_payload_vxlan_add_del_tunnel_reply,
                      CreateCmd>,
                  this));

    m_hw_item = wait();

    if (m_hw_item)
    {
        complete();
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
    Interface::DeleteCmd(item),
    m_ep(ep)
{
}

bool VxlanTunnel::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_ep == other.m_ep);
}

rc_t VxlanTunnel::DeleteCmd::issue(Connection &con)
{
    vapi_msg_vxlan_add_del_tunnel *req;

    req = vapi_alloc_vxlan_add_del_tunnel(con.ctx());
    req->payload.is_add = 0;
    req->payload.is_ipv6 = 0;
    to_bytes(m_ep.src, &req->payload.is_ipv6, req->payload.src_address);
    to_bytes(m_ep.dst, &req->payload.is_ipv6, req->payload.dst_address);
    req->payload.mcast_sw_if_index = ~0;
    req->payload.encap_vrf_id = 0;
    req->payload.decap_next_index = ~0;
    req->payload.vni = m_ep.vni;

    VAPI_CALL(vapi_vxlan_add_del_tunnel(
                  con.ctx(),
                  req,
                  Interface::create_callback<
                      vapi_payload_vxlan_add_del_tunnel_reply,
                      CreateCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);
    complete();

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
    vapi_msg_vxlan_tunnel_dump *req;

    req = vapi_alloc_vxlan_tunnel_dump(con.ctx());
    req->payload.sw_if_index = ~0;

    VAPI_CALL(vapi_vxlan_tunnel_dump(con.ctx(), req,
                                     DumpCmd::callback<DumpCmd>,
                                     this));

    wait();

    return rc_t::OK;
}

std::string VxlanTunnel::DumpCmd::to_string() const
{
    return ("Vpp-VxlanTunnels-Dump");
}
