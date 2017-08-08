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
#include "VppL3Binding.hpp"
#include "VppLldpGlobal.hpp"
#include "VppLldpBinding.hpp"
#include "VppArpProxyConfig.hpp"
#include "VppArpProxyBinding.hpp"
#include "VppIpUnnumbered.hpp"

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

void Uplink::configure_tap(const Route::prefix_t &pfx)
{
    /*
     * VPP will automatically apply the DHCP discovered address to the linux
     * side of the TAP interface
     */
    TapInterface itf("tuntap-0",
                     Interface::admin_state_t::UP,
                     Route::prefix_t::ZERO);
    VPP::OM::write(UPLINK_KEY, itf);

    /*
     * commit and L3 Config to the OM so this uplink owns the
     * subnet on the interface. If we don't have a representation
     * of the configured prefix in the OM, we'll sweep it from the
     * interface if we restart
     */
    SubInterface subitf(*m_uplink,
                        Interface::admin_state_t::UP,
                        m_vlan);
    L3Binding l3(subitf, pfx);
    OM::commit(UPLINK_KEY, l3);

    IpUnnumbered ipUnnumber(itf, subitf);
    VPP::OM::write(UPLINK_KEY, ipUnnumber);

    ArpProxyConfig arpProxyConfig(low(pfx), high(pfx));
    VPP::OM::write(UPLINK_KEY, arpProxyConfig);

    ArpProxyBinding arpProxyBinding(itf, arpProxyConfig);
    VPP::OM::write(UPLINK_KEY, arpProxyBinding);
}

void Uplink::handle_dhcp_event(DhcpConfig::EventsCmd *ec)
{
    /*
     * Create the TAP interface with the DHCP learn address.
     *  This allows all traffic punt to VPP to arrive at the TAP/agent.
     */
    std::lock_guard<DhcpConfig::EventsCmd> lg(*ec);

    for (auto &msg : *ec)
    {
        auto &payload = msg.get_payload();

        Route::prefix_t pfx(payload.is_ipv6,
                            payload.host_address,
                            payload.mask_width);

        configure_tap(pfx);
    }

    /*
     * VXLAN tunnels use the DHCP address as the source
     */
    m_vxlan.src = pfx.address();

    ec->flush();
}

void Uplink::configure(const std::string &fqdn)
{
    /*
     * Consruct the uplink physical, so we now 'own' it
     */
    Interface itf(m_iface,
                  Interface::type_t::ETHERNET,
                  Interface::admin_state_t::UP);
    OM::write(UPLINK_KEY, itf);

    /*
     * Find and save the interface this created
     */
    m_uplink = itf.singular();

    /**
     * Enable LLDP on this uplionk
     */
    LldpGlobal lg(fqdn, 5, 2);
    OM::write(UPLINK_KEY, lg);
    LldpBinding lb(*m_uplink, "uplink-interface");
    OM::write(UPLINK_KEY, lb);

    /*
     * now create the sub-interface on which control and data traffic from
     * the upstream leaf will arrive
     */
    SubInterface subitf(itf,
                        Interface::admin_state_t::UP,
                        m_vlan);
    OM::write(UPLINK_KEY, subitf);

    /**
     * Strip the fully qualified domain name of any domain name
     * to get just the hostname.
     */
    std::string hostname = fqdn;
    std::string::size_type n = hostname.find(".");
    if (n != std::string::npos)
    {
        hostname = hostname.substr(n);
    }

    /**
     * Configure DHCP on the uplink subinterface
     * We must use the MAC address of the uplink interface as the DHCP client-ID
     */
    DhcpConfig dc(subitf, hostname,
                  m_uplink->l2_address());
    OM::write(UPLINK_KEY, dc);

    /**
     * In the case of a agent restart, the DHCP process will already be complete
     * in VPP and we won't get notified. So let's cehck here if there alreay
     * exists an L3 config on the interface
     */
    std::deque<std::shared_ptr<L3Binding>> l3s = L3Binding::find(itf);

    if (l3s.size())
    {
        /*
         * there should only be one. we'll pick the first
         */
        std::shared_ptr<L3Binding> l3 = l3s.front();

        /*
         * Claim ownership.
         * VXLAN tunnels use the DHCP address as the source
         */
        OM::commit(UPLINK_KEY, *l3);

        configure_tap(l3->prefix());
        m_vxlan.src = l3->prefix().address();
    }
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
