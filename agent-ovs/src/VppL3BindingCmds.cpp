/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL3Binding.hpp"

DEFINE_VAPI_MSG_IDS_IP_API_JSON;

using namespace VPP;

L3Binding::BindCmd::BindCmd(HW::Item<bool> &item,
                           const handle_t &itf,
                           const Route::prefix_t &pfx):
    RpcCmd(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Binding::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Binding::BindCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_itf.value();
    payload.is_add = 1;
    payload.del_all = 0;

    m_pfx.to_vpp(&payload.is_ipv6,
                 payload.address,
                 &payload.address_length);

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string L3Binding::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}

L3Binding::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                               const handle_t &itf,
                               const Route::prefix_t &pfx):
    RpcCmd(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Binding::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Binding::UnbindCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_itf.value();
    payload.is_add = 0;
    payload.del_all = 0;

    m_pfx.to_vpp(&payload.is_ipv6,
                 payload.address,
                 &payload.address_length);

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    return rc_t::OK;
}

std::string L3Binding::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}

L3Binding::DumpV4Cmd::DumpV4Cmd(const handle_t &hdl):
    m_itf(hdl)
{
}

L3Binding::DumpV4Cmd::DumpV4Cmd(const DumpV4Cmd &d):
    m_itf(d.m_itf)
{
}

bool L3Binding::DumpV4Cmd::operator==(const DumpV4Cmd& other) const
{
    return (true);
}

rc_t L3Binding::DumpV4Cmd::issue(Connection &con)
{
    m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

    auto &payload = m_dump->get_request().get_payload();
    payload.sw_if_index = m_itf.value();
    payload.is_ipv6 = 0;

    VAPI_CALL(m_dump->execute());

    wait();

    return rc_t::OK;
}

std::string L3Binding::DumpV4Cmd::to_string() const
{
    return ("L3-binding-dump");
}
