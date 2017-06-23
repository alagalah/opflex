/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL3Config.hpp"

using namespace VPP;

L3Config::BindCmd::BindCmd(HW::Item<bool> &item,
                           const handle_t &itf,
                           const Route::prefix_t &pfx):
    RpcCmd(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Config::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Config::BindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_add_del_address *req;

    req = vapi_alloc_sw_interface_add_del_address(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.is_add = 1;

    m_pfx.to_vpp(&req->payload.is_ipv6,
                 req->payload.address,
                 &req->payload.address_length);

    vapi_sw_interface_add_del_address(
        con.ctx(), req,
        RpcCmd::callback<vapi_payload_sw_interface_add_del_address_reply,
                         BindCmd>,
        this);

    HW::Item<bool> res(true, wait());
    m_hw_item.update(res);

    return rc_t::OK;
}

std::string L3Config::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-config-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}

L3Config::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                               const handle_t &itf,
                               const Route::prefix_t &pfx):
    RpcCmd(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Config::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Config::UnbindCmd::issue(Connection &con)
{
    vapi_msg_sw_interface_add_del_address *req;

    req = vapi_alloc_sw_interface_add_del_address(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.is_add = 0;

    m_pfx.to_vpp(&req->payload.is_ipv6,
                 req->payload.address,
                 &req->payload.address_length);

    vapi_sw_interface_add_del_address(
        con.ctx(), req,
        RpcCmd::callback<vapi_payload_sw_interface_add_del_address_reply,
                         UnbindCmd>,
        this);

    HW::Item<bool> res(true, wait());
    m_hw_item.update(res);

    return rc_t::OK;
}

std::string L3Config::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-config-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}

L3Config::DumpV4Cmd::DumpV4Cmd()
{
}

bool L3Config::DumpV4Cmd::operator==(const DumpV4Cmd& other) const
{
    return (true);
}

rc_t L3Config::DumpV4Cmd::issue(Connection &con)
{
    vapi_msg_ip_fib_dump *req;

    req = vapi_alloc_ip_fib_dump(con.ctx());

    vapi_ip_fib_dump(con.ctx(), req, DumpCmd::callback<DumpV4Cmd>, this);

    wait();

    return rc_t::OK;
}

std::string L3Config::DumpV4Cmd::to_string() const
{
    return ("L3-config-dump");
}
