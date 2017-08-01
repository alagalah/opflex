/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppIpUnnumbered.hpp"

extern "C"
{
    #include "vpe.api.vapi.h"
}

using namespace VPP;

IpUnnumbered::ConfigCmd::ConfigCmd(HW::Item<bool> &item,
                                   const handle_t &itf,
                                   const handle_t &l3_itf):
    RpcCmd(item),
    m_itf(itf),
    m_l3_itf(l3_itf)
{
}

bool IpUnnumbered::ConfigCmd::operator==(const ConfigCmd& o) const
{
    return ((m_itf == o.m_itf) &&
            (m_l3_itf == o.m_l3_itf));
}

rc_t IpUnnumbered::ConfigCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_unnumbered *req;

    req = vapi_alloc_sw_interface_set_unnumbered(con.ctx());
    req->payload.is_add = 1;
    req->payload.sw_if_index = m_l3_itf.value();
    req->payload.unnumbered_sw_if_index = m_itf.value();

    VAPI_CALL(vapi_sw_interface_set_unnumbered(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_unnumbered_reply,
                                   ConfigCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string IpUnnumbered::ConfigCmd::to_string() const
{
    std::ostringstream s;
    s << "IP-unnumberd-config: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " l3-itf:" << m_l3_itf.to_string();

    return (s.str());
}

IpUnnumbered::UnconfigCmd::UnconfigCmd(HW::Item<bool> &item,
                                   const handle_t &itf,
                                   const handle_t &l3_itf):
    RpcCmd(item),
    m_itf(itf),
    m_l3_itf(l3_itf)
{
}

bool IpUnnumbered::UnconfigCmd::operator==(const UnconfigCmd& o) const
{
    return ((m_itf == o.m_itf) &&
            (m_l3_itf == o.m_l3_itf));
}

rc_t IpUnnumbered::UnconfigCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_set_unnumbered *req;

    req = vapi_alloc_sw_interface_set_unnumbered(con.ctx());
    req->payload.is_add = 0;
    req->payload.sw_if_index = m_l3_itf.value();
    req->payload.unnumbered_sw_if_index = m_itf.value();

    VAPI_CALL(vapi_sw_interface_set_unnumbered(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_unnumbered_reply,
                                   ConfigCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string IpUnnumbered::UnconfigCmd::to_string() const
{
    std::ostringstream s;
    s << "IP-unnumberd-unconfig: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " l3-itf:" << m_l3_itf.to_string();

    return (s.str());
}
