/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppVxlanTunnel.hpp"
#include "VppRoute.hpp"
#include "VppCmd.hpp"

using namespace VPP;

const std::string VXLAN_TUNNEL_NAME = "vxlan-tunnel-itf";


/**
 * A DB of al the interfaces, key on the name
 */
InstDB<VxlanTunnel::endpoint_t, VxlanTunnel> VxlanTunnel::m_db;

VxlanTunnel::endpoint_t::endpoint_t(const boost::asio::ip::address &src,
                                    const boost::asio::ip::address &dst,
                                    uint32_t vni):
    src(src),
    dst(dst),
    vni(vni)
{
}

VxlanTunnel::endpoint_t::endpoint_t():
    src(),
    dst(),
    vni(0)
{
}

bool VxlanTunnel::endpoint_t::operator<(const VxlanTunnel::endpoint_t &o) const
{
    if (src < o.src) return true;
    if (dst < o.dst) return true;
    if (vni < o.vni) return true;

    return false;
}

std::string VxlanTunnel::endpoint_t::to_string() const
{
    std::ostringstream s;

    s << "ep:["
      << "src:" << src.to_string()
      << " dst:" << dst.to_string()
      << " vni:" << vni
      << "]";
 
    return (s.str());
}

std::string VxlanTunnel::mk_name(const boost::asio::ip::address &src,
                                 const boost::asio::ip::address &dst,
                                 uint32_t vni)
{
    std::ostringstream s;

    s << VXLAN_TUNNEL_NAME
      << "-"
      << src
      << "-"
      << dst
      << ":"
      << vni;

    return (s.str());
}

/**
 * Construct a new object matching the desried state
 */
VxlanTunnel::VxlanTunnel(const boost::asio::ip::address &src,
                         const boost::asio::ip::address &dst,
                         uint32_t vni):
    Interface(mk_name(src, dst, vni),
              Interface::type_t::VXLAN,
              Interface::admin_state_t::UP),
    m_tep(src, dst, vni)
{
}

VxlanTunnel::VxlanTunnel(const VxlanTunnel& o):
    Interface(o),
    m_tep(o.m_tep)
{
}

const handle_t & VxlanTunnel::handle() const
{
    return (m_hdl.data());
}

void VxlanTunnel::sweep()
{
    if (m_hdl)
    {
        HW::enqueue(new DeleteCmd(m_hdl, m_tep.src, m_tep.dst, m_tep.vni));
    }
    HW::write();
}

VxlanTunnel::~VxlanTunnel()
{
    sweep();

    // not in the DB anymore.
    Interface::release();
    m_db.release(m_tep, this);
}

std::string VxlanTunnel::to_string() const
{
    std::ostringstream s;
    s << "vxlan-tunnel: "
      << m_hdl.to_string()
      << " "
      << m_tep.to_string();

    return (s.str());
}

void VxlanTunnel::update(const VxlanTunnel &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (!m_hdl)
    {
        HW::enqueue(new CreateCmd(m_hdl, m_tep.src, m_tep.dst, m_tep.vni));
    }
}

std::shared_ptr<VxlanTunnel> VxlanTunnel::find_or_add(const VxlanTunnel &temp)
{
    std::shared_ptr<VxlanTunnel> sp = m_db.find_or_add(temp.m_tep, temp);

    Interface::insert(temp, sp);

    return (sp);
}

std::shared_ptr<VxlanTunnel> VxlanTunnel::find(const VxlanTunnel &temp)
{
    return (m_db.find(temp.m_tep));
}
