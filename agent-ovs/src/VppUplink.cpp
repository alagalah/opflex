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
    /*
     * Create the TAP interface with the DHCP learn address.
     *  This allows all traffic punt to VPP to arrive at the TAP/agent.
     */
    vapi_payload_dhcp_compl_event dhcp_data;

    cmd->pop(dhcp_data);

    Route::prefix_t pfx(dhcp_data.is_ipv6,
                        dhcp_data.host_address,
                        dhcp_data.mask_width);

    ControlInterface itf("tuntap0",
                         Interface::type_t::TAP,
                         Interface::admin_state_t::UP,
                         pfx);
    VPP::OM::write(UPLINK_KEY, itf);
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
}

void Uplink::set(const std::string &uplink,
                 uint16_t uplink_vlan,
                 const std::string &encap_name,
                 const boost::asio::ip::address &remote_ip,
                 uint16_t port)
{
    m_type = VXLAN;
    m_vxlan.dst = remote_ip;
    m_iface = uplink;
    m_vlan = uplink_vlan;
}

void Uplink::set(const std::string &uplink,
                 uint16_t uplink_vlan,
                 const std::string &encap_name)
{
    m_type = VLAN;
    m_iface = uplink;
    m_vlan = uplink_vlan;
}
