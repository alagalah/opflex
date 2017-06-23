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
    // finally... call VPP
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
    // finally... call VPP
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

