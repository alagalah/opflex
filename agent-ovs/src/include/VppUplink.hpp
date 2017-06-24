/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_UPLINK_H__
#define __VPP_UPLINK_H__

#include "VppVxlanTunnel.hpp"
#include "VppDhcpConfig.hpp"

namespace VPP
{
    /*
     * forward declarations to reduce compile time coupling
     */
    class Interface;

    /**
     * A description of the uplink interface. 
     *  Can be one of VLAN< VXLAN or iVXLAN
     */
    class Uplink
    {
    public:
        enum uplink_type_t
        {
            VXLAN,
            VLAN
        };

        Uplink();

        /**
         * Given the VNID, create aninterface of the appropriate type
         */
        VPP::Interface *mk_interface(const std::string &uuid, uint32_t vnid);

        void set(const std::string &uplink,
                 uint16_t vlan,
                 Route::prefix_t &uplink_prefix,
                 const std::string &name,
                 const boost::asio::ip::address &ip,
                 uint16_t port);

        /**
         * make the control channel/interfaces
         */
        void configure();

        void handle_dhcp_event(DhcpConfig::EventsCmd *cmd);

    private:
        uplink_type_t m_type;

        VPP::VxlanTunnel::endpoint_t m_vxlan;

        /**
         * A reference to the uplink physical insterface in the OM
         */
        std::shared_ptr<Interface> m_uplink;

        /**
         * Paramters saved from the set-properties call
         */
        std::string m_iface;
        uint16_t m_vlan;
        Route::prefix_t m_prefix;
    };
};

#endif
