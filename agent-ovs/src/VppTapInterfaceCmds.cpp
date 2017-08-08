/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "VppCmd.hpp"
#include "VppTapInterface.hpp"

#include <vapi/tap.api.vapi.hpp>

using namespace VPP;

TapInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                       const std::string &name,
                                       Route::prefix_t &prefix,
                                       const l2_address_t &l2_address):
    RpcCmd(item),
    m_name(name),
    m_prefix(prefix),
    m_l2_address(l2_address)
{
}

rc_t TapInterface::CreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    memset(payload.tap_name, 0,
                    sizeof(payload.tap_name));
    memcpy(payload.tap_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.tap_name)));
    if (m_prefix != Route::prefix_t::ZERO) {
        if (m_prefix.address().is_v6()) {
            m_prefix.to_vpp(&payload.ip6_address_set,
                 payload.ip6_address,
                 &payload.ip6_mask_width);
        } else {
            m_prefix.to_vpp(&payload.ip4_address_set,
                 payload.ip4_address,
                 &payload.ip4_mask_width);
           payload.ip4_address_set = 1;
       }
    }

    if (m_l2_address != l2_address_t::ZERO) {
       m_l2_address.to_bytes(payload.mac_address, 6);
    } else {
       payload.use_random_mac = 1;
    }

    VAPI_CALL(req.execute());

    m_hw_item =  wait();

    return rc_t::OK;
}

std::string TapInterface::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-intf-create: " << m_hw_item.to_string()
      << " ip-prefix:" << m_prefix.to_string(); 

    return (s.str());
}

bool TapInterface::CreateCmd::operator==(const CreateCmd& other) const
{
    return (CreateCmd::operator==(other));
}

TapInterface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item):
    RpcCmd(item)
{
}

rc_t TapInterface::DeleteCmd::issue(Connection &con)
{
    // finally... call VPP

    return rc_t::OK;
}
std::string TapInterface::DeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

bool TapInterface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

TapInterface::DumpCmd::DumpCmd()
{
}

bool TapInterface::DumpCmd::operator==(const DumpCmd& other) const
{
    return (true);
}

rc_t TapInterface::DumpCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    VAPI_CALL(req.execute());

    wait();

    return rc_t::OK;
}

std::string TapInterface::DumpCmd::to_string() const
{
    return ("tap-itf-dump");
}
