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

extern "C"
{
    #include "vxlan.api.vapi.h"
}

using namespace VPP;

VxlanTunnel::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                  const boost::asio::ip::address &src,
                                  const boost::asio::ip::address &dst,
                                  uint32_t vni):
    RpcCmd(item),
    m_src(src),
    m_dst(dst),
    m_vni(vni)
{
}

bool VxlanTunnel::CreateCmd::operator==(const CreateCmd& other) const
{
    return ((m_src == other.m_src) &&
            (m_dst == other.m_dst) &&
            (m_vni == other.m_vni));
}

rc_t VxlanTunnel::CreateCmd::issue(Connection &con)
{
    vapi_msg_vxlan_add_del_tunnel *req;

    req = vapi_alloc_vxlan_add_del_tunnel(con.ctx());
    req->payload.is_add = 1;
    req->payload.is_ipv6 = 0;
    to_bytes(m_src, &req->payload.is_ipv6, req->payload.src_address);
    to_bytes(m_dst, &req->payload.is_ipv6, req->payload.dst_address);
    req->payload.mcast_sw_if_index = ~0;
    req->payload.encap_vrf_id = 0;
    req->payload.decap_next_index = ~0;
    req->payload.vni = m_vni;

    VAPI_CALL(vapi_vxlan_add_del_tunnel(
                  con.ctx(),
                  req,
                  Interface::create_callback<
                      vapi_payload_vxlan_add_del_tunnel_reply,
                      CreateCmd>,
                  this));

    m_hw_item = wait();

    return rc_t::OK;
}

std::string VxlanTunnel::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "vxlan-tunnel-create: " << m_hw_item.to_string()
      << " src:" << m_src.to_string()
      << " dst:" << m_dst.to_string()
      << " vni:" << m_vni;

    return (s.str());
}

VxlanTunnel::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item,
                                  const boost::asio::ip::address &src,
                                  const boost::asio::ip::address &dst,
                                  uint32_t vni):
    RpcCmd(item),
    m_src(src),
    m_dst(dst),
    m_vni(vni)
{
}

bool VxlanTunnel::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return ((m_src == other.m_src) &&
            (m_dst == other.m_dst) &&
            (m_vni == other.m_vni));
}

rc_t VxlanTunnel::DeleteCmd::issue(Connection &con)
{
    vapi_msg_vxlan_add_del_tunnel *req;

    req = vapi_alloc_vxlan_add_del_tunnel(con.ctx());
    req->payload.is_add = 0;
    req->payload.is_ipv6 = 0;
    to_bytes(m_src, &req->payload.is_ipv6, req->payload.src_address);
    to_bytes(m_dst, &req->payload.is_ipv6, req->payload.dst_address);
    req->payload.mcast_sw_if_index = ~0;
    req->payload.encap_vrf_id = 0;
    req->payload.decap_next_index = ~0;
    req->payload.vni = m_vni;

    VAPI_CALL(vapi_vxlan_add_del_tunnel(
                  con.ctx(),
                  req,
                  Interface::create_callback<
                      vapi_payload_vxlan_add_del_tunnel_reply,
                      CreateCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    return (rc_t::OK);
}

std::string VxlanTunnel::DeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "vxlan-tunnel-delete: " << m_hw_item.to_string()
      << " src:" << m_src.to_string()
      << " dst:" << m_dst.to_string()
      << " vni:" << m_vni;

    return (s.str());
}

