/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppLldpGlobal.hpp"

using namespace VPP;

LldpGlobal::ConfigCmd::ConfigCmd(HW::Item<bool> &item,
                                 const std::string &system_name,
                                 uint32_t tx_hold,
                                 uint32_t tx_interval):
    RpcCmd(item),
    m_system_name(system_name),
    m_tx_hold(tx_hold),
    m_tx_interval(tx_interval)
{
}

bool LldpGlobal::ConfigCmd::operator==(const ConfigCmd& other) const
{
    return (m_system_name == other.m_system_name);
}

rc_t LldpGlobal::ConfigCmd::issue(Connection &con)
{
    vapi_msg_lldp_config *req;

    req = vapi_alloc_lldp_config(con.ctx());
    req->payload.tx_hold = m_tx_hold;
    req->payload.tx_interval = m_tx_interval;
    
    memcpy(req->payload.system_name,
           m_system_name.c_str(),
           std::min(sizeof(req->payload.system_name),
                           m_system_name.length()));

    VAPI_CALL(vapi_lldp_config(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_lldp_config_reply,
                                   ConfigCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string LldpGlobal::ConfigCmd::to_string() const
{
    std::ostringstream s;
    s << "Lldp-global-config: " << m_hw_item.to_string()
      << " system_name:" << m_system_name
      << " tx-hold:" << m_tx_hold
      << " tx-interval:" << m_tx_interval;

    return (s.str());
}