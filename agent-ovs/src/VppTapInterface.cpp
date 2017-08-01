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
    #include "vpe.api.vapi.h"
}

using namespace VPP;

/**
 * Construct a new object matching the desried state
 */
TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix):
    Interface(name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address_t::ZERO)
{
}

TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix,
                           const l2_address_t &l2_address):
    Interface(name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address)
{
}

TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           const l2_address_t &l2_address):
    Interface(name, type_t::TAP, state),
    m_prefix(Route::prefix_t::ZERO),
    m_l2_address(l2_address)
{
}

TapInterface::TapInterface(const handle_t &hdl,
                           const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix):
    Interface(hdl, l2_address_t::ZERO, name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address_t::ZERO)
{
}

TapInterface::~TapInterface()
{
    sweep();
    release();
}

TapInterface::TapInterface(const TapInterface& o):
    Interface(o),
    m_prefix(o.m_prefix),
    m_l2_address(o.m_l2_address)
{
}

std::queue<Cmd*> &  TapInterface::mk_create_cmd(std::queue<Cmd*> &q)
{
    q.push(new CreateCmd(m_hdl, name(), m_prefix, m_l2_address));

    return (q);
}

std::queue<Cmd*> &  TapInterface::mk_delete_cmd(std::queue<Cmd*> &q)
{
    q.push(new DeleteCmd(m_hdl));

    return (q);
}

std::shared_ptr<TapInterface> TapInterface::singular() const
{
    return std::dynamic_pointer_cast<TapInterface>(singular_i());
}

std::shared_ptr<Interface> TapInterface::singular_i() const
{
    return m_db.find_or_add(name(), *this);
}

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
