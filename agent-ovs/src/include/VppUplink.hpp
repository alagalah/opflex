/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Copyright (c) 2014-2016 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_UPLINK_H__
#define __VPP_UPLINK_H__

#include "VppInterface.hpp"
#include "VppVxlanTunnel.hpp"

namespace VPP
{
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
        VPP::Interface makeInterface(uint32_t vnid);

        void set(const std::string &uplink,
                 uint16_t vlan,
                 const std::string &name,
                 const boost::asio::ip::address &ip,
                 uint16_t port);

    private:
        uplink_type_t m_type;

        VPP::VxlanTunnel::endpoint_t m_vxlan;
    };
};

#endif
