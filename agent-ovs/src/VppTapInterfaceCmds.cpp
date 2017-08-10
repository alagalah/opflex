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

extern "C"
{
    #include "tap.api.vapi.h"
}

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
    vapi_msg_tap_connect *req;

    req = vapi_alloc_tap_connect(con.ctx());
    memset(req->payload.tap_name, 0,
                    sizeof(req->payload.tap_name));
    memcpy(req->payload.tap_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(req->payload.tap_name)));
    if (m_prefix != Route::prefix_t::ZERO) {
        if (m_prefix.address().is_v6()) {
            m_prefix.to_vpp(&req->payload.ip6_address_set,
                 req->payload.ip6_address,
                 &req->payload.ip6_mask_width);
        } else {
            m_prefix.to_vpp(&req->payload.ip4_address_set,
                 req->payload.ip4_address,
                 &req->payload.ip4_mask_width);
           req->payload.ip4_address_set = 1;
       }
    }

    if (m_l2_address != l2_address_t::ZERO) {
       m_l2_address.to_bytes(req->payload.mac_address, 6);
    } else {
       req->payload.use_random_mac = 1;
    }

    VAPI_CALL(vapi_tap_connect(con.ctx(), req,
                               Interface::create_callback<
                                   vapi_payload_tap_connect_reply,
                                   CreateCmd>,
                               this));
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
    vapi_msg_sw_interface_tap_dump *req;

    req = vapi_alloc_sw_interface_tap_dump(con.ctx());

    VAPI_CALL(vapi_sw_interface_tap_dump(con.ctx(), req,
                                         DumpCmd::callback<DumpCmd>,
                                         this));

    wait();

    return rc_t::OK;
}

std::string TapInterface::DumpCmd::to_string() const
{
    return ("tap-itf-dump");
}
