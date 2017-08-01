/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppArpProxyConfig.hpp"

extern "C"
{
    #include "vpe.api.vapi.h"
}

using namespace VPP;

ArpProxyConfig::ConfigCmd::ConfigCmd(HW::Item<bool> &item,
                                     const boost::asio::ip::address_v4 &low,
                                     const boost::asio::ip::address_v4 &high):
    RpcCmd(item),
    m_low(low),
    m_high(high)
{
}

bool ArpProxyConfig::ConfigCmd::operator==(const ConfigCmd& o) const
{
    return ((m_low == o.m_low) &&
            (m_high == o.m_high));
}

rc_t ArpProxyConfig::ConfigCmd::issue(Connection &con)
{
    vapi_msg_proxy_arp_add_del *req;

    req = vapi_alloc_proxy_arp_add_del(con.ctx());
    req->payload.is_add = 1;

    std::copy_n(std::begin(m_low.to_bytes()),
                m_low.to_bytes().size(),
                req->payload.low_address);
    std::copy_n(std::begin(m_high.to_bytes()),
                m_high.to_bytes().size(),
                req->payload.hi_address);

    VAPI_CALL(vapi_proxy_arp_add_del(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_proxy_arp_add_del_reply,
                                   ConfigCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string ArpProxyConfig::ConfigCmd::to_string() const
{
    std::ostringstream s;
    s << "ARP-proxy-config: " << m_hw_item.to_string()
      << " low:" << m_low.to_string()
      << " high:" << m_high.to_string();

    return (s.str());
}

ArpProxyConfig::UnconfigCmd::UnconfigCmd(HW::Item<bool> &item,
                                         const boost::asio::ip::address_v4 &low,
                                         const boost::asio::ip::address_v4 &high):
    RpcCmd(item),
    m_low(low),
    m_high(high)
{
}

bool ArpProxyConfig::UnconfigCmd::operator==(const UnconfigCmd& o) const
{
    return ((m_low == o.m_low) &&
            (m_high == o.m_high));
}

rc_t ArpProxyConfig::UnconfigCmd::issue(Connection &con)
{
    vapi_msg_proxy_arp_add_del *req;

    req = vapi_alloc_proxy_arp_add_del(con.ctx());
    req->payload.is_add = 0;

    std::copy_n(std::begin(m_low.to_bytes()),
                m_low.to_bytes().size(),
                req->payload.low_address);
    std::copy_n(std::begin(m_high.to_bytes()),
                m_high.to_bytes().size(),
                req->payload.hi_address);

    VAPI_CALL(vapi_proxy_arp_add_del(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_proxy_arp_add_del_reply,
                                   UnconfigCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string ArpProxyConfig::UnconfigCmd::to_string() const
{
    std::ostringstream s;
    s << "ARP-proxy-unconfig: " << m_hw_item.to_string()
      << " low:" << m_low.to_string()
      << " high:" << m_high.to_string();

    return (s.str());
}
