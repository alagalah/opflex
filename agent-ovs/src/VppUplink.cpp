/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Copyright (c) 2014-2016 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppUplink.hpp"

using namespace VPP;

Uplink::Uplink()
{
}

VPP::Interface Uplink::makeInterface(uint32_t vnid)
{
    switch (m_type)
    {
    case VXLAN:
        return VxlanTunnel(m_vxlan.src,
                           m_vxlan.dst,
                           vnid);
    case VLAN:
        break;
    }
}

void Uplink::set(const std::string &uplink,
                 uint16_t vlan,
                 const std::string &name,
                 const boost::asio::ip::address &ip,
                 uint16_t port)
{
    m_vxlan.src = ip;
}
