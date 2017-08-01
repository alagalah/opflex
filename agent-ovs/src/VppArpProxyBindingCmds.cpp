/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppArpProxyBinding.hpp"

using namespace VPP;

ArpProxyBinding::BindCmd::BindCmd(HW::Item<bool> &item,
                                  const handle_t &itf):
    RpcCmd(item),
    m_itf(itf)
{
}

bool ArpProxyBinding::BindCmd::operator==(const BindCmd& other) const
{
    return (m_itf == other.m_itf);
}

rc_t ArpProxyBinding::BindCmd::issue(Connection &con)
{
    vapi_msg_proxy_arp_intfc_enable_disable *req;

    req = vapi_alloc_proxy_arp_intfc_enable_disable(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.enable_disable = 1;

    VAPI_CALL(vapi_proxy_arp_intfc_enable_disable(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_proxy_arp_intfc_enable_disable_reply, BindCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string ArpProxyBinding::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "ARP-proxy-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string();

    return (s.str());
}

ArpProxyBinding::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                                 const handle_t &itf):
    RpcCmd(item),
    m_itf(itf)
{
}

bool ArpProxyBinding::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return (m_itf == other.m_itf);
}

rc_t ArpProxyBinding::UnbindCmd::issue(Connection &con)
{
    vapi_msg_proxy_arp_intfc_enable_disable *req;

    req = vapi_alloc_proxy_arp_intfc_enable_disable(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.enable_disable = 0;

    VAPI_CALL(vapi_proxy_arp_intfc_enable_disable(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_proxy_arp_intfc_enable_disable_reply, BindCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    return rc_t::OK;
}

std::string ArpProxyBinding::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "ARP-proxy-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string();

    return (s.str());
}
