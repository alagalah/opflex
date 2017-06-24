/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL2Config.hpp"

using namespace VPP;

L2Config::BindCmd::BindCmd(HW::Item<bool> &item,
                           const handle_t &itf,
                           uint32_t bd,
                           bool is_bvi):
    RpcCmd(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Config::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Config::BindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_l2_bridge* req;
    
    req = vapi_alloc_sw_interface_set_l2_bridge(con.ctx());
    req->payload.rx_sw_if_index = m_itf.value();
    req->payload.bd_id = m_bd;
    req->payload.shg = 0;
    req->payload.bvi = m_is_bvi;
    req->payload.enable = 1;

    vapi_sw_interface_set_l2_bridge(con.ctx(),
                                    req,
                                    RpcCmd::callback<vapi_payload_sw_interface_set_l2_bridge_reply,
                                                     BindCmd>,
                                    this);

    m_hw_item.set(wait());
                                            
    return (rc_t::OK);
}

std::string L2Config::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-config-BD-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd;

    return (s.str());
}

L2Config::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                               const handle_t &itf,
                               uint32_t bd,
                               bool is_bvi):
    RpcCmd(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Config::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Config::UnbindCmd::issue(Connection &con)
{
    // finally... call VPP
    return (rc_t::OK);
}

std::string L2Config::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-config-BD-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd;

    return (s.str());
}
