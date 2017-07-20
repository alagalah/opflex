/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL2Binding.hpp"

using namespace VPP;

L2Binding::BindCmd::BindCmd(HW::Item<bool> &item,
                           const handle_t &itf,
                           uint32_t bd,
                           bool is_bvi):
    RpcCmd(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Binding::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Binding::BindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_l2_bridge* req;
    
    req = vapi_alloc_sw_interface_set_l2_bridge(con.ctx());
    req->payload.rx_sw_if_index = m_itf.value();
    req->payload.bd_id = m_bd;
    req->payload.shg = 0;
    req->payload.bvi = m_is_bvi;
    req->payload.enable = 1;

    VAPI_CALL(vapi_sw_interface_set_l2_bridge(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_l2_bridge_reply,
                  BindCmd>,
                  this));

    m_hw_item.set(wait());
                                            
    return (rc_t::OK);
}

std::string L2Binding::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-config-BD-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd;

    return (s.str());
}

L2Binding::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                               const handle_t &itf,
                               uint32_t bd,
                               bool is_bvi):
    RpcCmd(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Binding::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Binding::UnbindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_l2_bridge* req;

    req = vapi_alloc_sw_interface_set_l2_bridge(con.ctx());
    req->payload.rx_sw_if_index = m_itf.value();
    req->payload.bd_id = m_bd;
    req->payload.shg = 0;
    req->payload.bvi = m_is_bvi;
    req->payload.enable = 0;

    VAPI_CALL(vapi_sw_interface_set_l2_bridge(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_l2_bridge_reply,
                  BindCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    return (rc_t::OK);
}

std::string L2Binding::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-config-BD-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd;

    return (s.str());
}
