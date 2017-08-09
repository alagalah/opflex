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
#include "VppLogger.hpp"

using namespace VPP;

const std::string VXLAN_TUNNEL_NAME = "vxlan-tunnel-itf";

VxlanTunnel::EventHandler VxlanTunnel::m_evh;

/**
 * A DB of all VxlanTunnels
 * this does not register as a listener for replay events, since the tunnels
 * are also in the base-class Interface DB and so will be poked from there.
 */
SingularDB<VxlanTunnel::endpoint_t, VxlanTunnel> VxlanTunnel::m_db;

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

bool VxlanTunnel::endpoint_t::operator==(const endpoint_t& other) const
{
    return ((src == other.src) &&
            (dst == other.dst) &&
            (vni == other.vni));
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

std::ostream & VPP::operator<<(std::ostream &os, const VxlanTunnel::endpoint_t &ep)
{
    os << ep.to_string();

    return (os);
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

VxlanTunnel::VxlanTunnel(const boost::asio::ip::address &src,
                         const boost::asio::ip::address &dst,
                         uint32_t vni):
    Interface(mk_name(src, dst, vni),
              Interface::type_t::VXLAN,
              Interface::admin_state_t::UP),
    m_tep(src, dst, vni)
{
}

VxlanTunnel::VxlanTunnel(const handle_t &hdl,
                         const boost::asio::ip::address &src,
                         const boost::asio::ip::address &dst,
                         uint32_t vni):
    Interface(hdl,
              l2_address_t::ZERO,
              mk_name(src, dst, vni),
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
        HW::enqueue(new DeleteCmd(m_hdl, m_tep));
    }
    HW::write();
}

void VxlanTunnel::replay()
{
   if (m_hdl)
   {
       HW::enqueue(new CreateCmd(m_hdl, name(), m_tep));
   }
}

VxlanTunnel::~VxlanTunnel()
{
    sweep();

    /*
     * release from both DBs
     */
    release();
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
        HW::enqueue(new CreateCmd(m_hdl, name(), m_tep));
    }
}

std::shared_ptr<VxlanTunnel> VxlanTunnel::find_or_add(const VxlanTunnel &temp)
{
    /*
     * a VXLAN tunnel needs to be in both the interface-find-by-name
     * and the VxlanTunnel-find-by-endpoint singular databases
     */
    std::shared_ptr<VxlanTunnel> sp;

    sp = m_db.find_or_add(temp.m_tep, temp);

    Interface::m_db.add(temp.name(), sp);

    return (sp);
}

std::shared_ptr<VxlanTunnel> VxlanTunnel::singular() const
{
    return (find_or_add(*this));
}

std::shared_ptr<Interface> VxlanTunnel::singular_i() const
{
    return find_or_add(*this);
}

void VxlanTunnel::dump(std::ostream &os)
{
    m_db.dump(os);
}

void VxlanTunnel::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    /*
     * dump VPP current states
     */
    std::shared_ptr<VxlanTunnel::DumpCmd> cmd(new VxlanTunnel::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    for (auto &record : *cmd)
    {
        auto &payload = record.get_payload();
        handle_t hdl(payload.sw_if_index);
        boost::asio::ip::address src = from_bytes(payload.is_ipv6,
                                                  payload.src_address);
        boost::asio::ip::address dst = from_bytes(payload.is_ipv6,
                                                  payload.dst_address);

        VxlanTunnel vt(hdl, src, dst, payload.vni);

        BOOST_LOG_SEV(logger(), levels::debug) << "dump: " << vt.to_string();

        OM::commit(key, vt);
    }
}

VxlanTunnel::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"vxlan"}, "VXLAN Tunnels", this);
}

void VxlanTunnel::EventHandler::handle_replay()
{
    // replay is handled from the interface DB
}

dependency_t VxlanTunnel::EventHandler::order() const
{
    return (dependency_t::TUNNEL);
}

void VxlanTunnel::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
