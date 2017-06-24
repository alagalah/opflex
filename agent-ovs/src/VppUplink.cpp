/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Copyright (c) 2014-2016 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppUplink.hpp"
#include "VppInterface.hpp"
#include "VppSubInterface.hpp"
#include "VppL3Config.hpp"

using namespace VPP;

static const std::string UPLINK_KEY = "__uplink__";

Uplink::Uplink()
{
}

VPP::Interface* Uplink::mk_interface(const std::string &uuid,
                                     uint32_t vnid)
{
    switch (m_type)
    {
    case VXLAN:
    {
        VxlanTunnel *vt = new VxlanTunnel(m_vxlan.src, m_vxlan.dst, vnid);

        VPP::OM::write(uuid, *vt);

        return (vt);
    }
    case VLAN:
    {
        SubInterface *sb = new SubInterface(*m_uplink, Interface::admin_state_t::UP, vnid);

        VPP::OM::write(uuid, *sb);

        return (sb);
    }
    }
}

void Uplink::handle_dhcp_event(DhcpConfig::EventsCmd *cmd)
{
}

void Uplink::configure()
{
    /*
     * Consruct the uplink physical, so we now 'own' it
     */
    Interface itf(m_iface,
                  Interface::type_t::AFPACKET,
                  Interface::admin_state_t::UP);
    VPP::OM::write(UPLINK_KEY, itf);

    /*
     * Find and save the interface this created
     */
    m_uplink = Interface::find(itf);

    /*
     * now create the sub-interface on which control and data traffic from
     * the upstream leaf will arrive
     */
    SubInterface subitf(itf,
                        Interface::admin_state_t::UP,
                        m_vlan);
    VPP::OM::write(UPLINK_KEY, subitf);

    /**
     * Configure DHCP on the uplink subinterface
     */
    DhcpConfig dc(itf, "agent-opflex");
    VPP::OM::write(UPLINK_KEY, dc);
    
    /**
     * Add the prefix to the control interface
     */
    /* L3Config l3(itf, m_prefix); */
    /* VPP::OM::write(UPLINK_KEY, l3); */

    /*
     * Just for fun... dump configs
     */
    std::shared_ptr<L3Config::DumpV4Cmd> cmd(new L3Config::DumpV4Cmd());

    HW::enqueue(cmd);
    HW::write();

    L3Config::DumpV4Cmd::details_type data;

    while (cmd->pop(data))
    {
        Route::prefix_t pfx(0, data.address, data.address_length);

        LOG(ovsagent::INFO) << "dump: " << pfx.to_string();
    }
}

void Uplink::set(const std::string &uplink,
                 uint16_t uplink_vlan,
                 Route::prefix_t &uplink_prefix,
                 const std::string &encap_name,
                 const boost::asio::ip::address &remote_ip,
                 uint16_t port)
{
    m_type = VXLAN;
    m_vxlan.dst = remote_ip;
    m_vxlan.src = uplink_prefix.address();
    m_iface = uplink;
    m_vlan = uplink_vlan;
    m_prefix = uplink_prefix;
}
