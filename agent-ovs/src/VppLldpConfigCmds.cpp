/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppLldpConfig.hpp"

DEFINE_VAPI_MSG_IDS_LLDP_API_JSON;

using namespace VPP;

LldpConfig::BindCmd::BindCmd(HW::Item<bool> &item,
                             const handle_t &itf,
                             const std::string &port_desc):
    RpcCmd(item),
    m_itf(itf),
    m_port_desc(port_desc)
{
}

bool LldpConfig::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_port_desc == other.m_port_desc));
}

rc_t LldpConfig::BindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_lldp *req;

    req = vapi_alloc_sw_interface_set_lldp(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.enable = 1;
    
    memcpy(req->payload.port_desc,
           m_port_desc.c_str(),
           std::min(sizeof(req->payload.port_desc),
                           m_port_desc.length()));

    VAPI_CALL(vapi_sw_interface_set_lldp(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_lldp_reply,
                                   BindCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string LldpConfig::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "Lldp-config-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " port_desc:" << m_port_desc;

    return (s.str());
}

LldpConfig::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                                 const handle_t &itf):
    RpcCmd(item),
    m_itf(itf)
{
}

bool LldpConfig::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return (m_itf == other.m_itf);
}

rc_t LldpConfig::UnbindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_lldp *req;

    req = vapi_alloc_sw_interface_set_lldp(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.enable = 0;
    
    VAPI_CALL(vapi_sw_interface_set_lldp(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_lldp_reply,
                                   BindCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    return rc_t::OK;
}

std::string LldpConfig::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "Lldp-config-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string();

    return (s.str());
}
