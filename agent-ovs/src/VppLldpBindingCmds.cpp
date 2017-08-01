/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppLldpBinding.hpp"

DEFINE_VAPI_MSG_IDS_LLDP_API_JSON;

using namespace VPP;

LldpBinding::BindCmd::BindCmd(HW::Item<bool> &item,
                              const handle_t &itf,
                              const std::string &port_desc):
    RpcCmd(item),
    m_itf(itf),
    m_port_desc(port_desc)
{
}

bool LldpBinding::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_port_desc == other.m_port_desc));
}

rc_t LldpBinding::BindCmd::issue(Connection &con)
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

std::string LldpBinding::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "Lldp-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " port_desc:" << m_port_desc;

    return (s.str());
}

LldpBinding::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                                 const handle_t &itf):
    RpcCmd(item),
    m_itf(itf)
{
}

bool LldpBinding::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return (m_itf == other.m_itf);
}

rc_t LldpBinding::UnbindCmd::issue(Connection &con)
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

std::string LldpBinding::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "Lldp-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string();

    return (s.str());
}
