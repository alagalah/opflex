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
 * A DB of all the tap-interfaces, key on the name
 */
InstDB<const std::string, TapInterface> TapInterface::m_db;

/**
 * Construct a new object matching the desried state
 */
TapInterface::TapInterface(const std::string &name,
                                   type_t type,
                                   admin_state_t state,
                                   Route::prefix_t prefix):
    Interface(name, type, state),
    m_prefix(prefix)
{
}

TapInterface::~TapInterface()
{
    sweep();

    // not in the DB anymore.
    Interface::release();
    m_db.release(name(), this);
}

TapInterface::TapInterface(const TapInterface& o):
    Interface(o),
    m_prefix(o.m_prefix)
{
}

std::shared_ptr<TapInterface> TapInterface::find_or_add(const TapInterface &temp)
{
    std::shared_ptr<TapInterface> sp = m_db.find_or_add(temp.key(), temp);

    Interface::insert(temp, sp);

    return (sp);
}

std::shared_ptr<TapInterface> TapInterface::find(const TapInterface &temp)
{
    return (m_db.find(temp.name()));
}

Cmd* TapInterface::mk_create_cmd()
{
    return (new CreateCmd(m_hdl, name(), m_prefix));
}

Cmd* TapInterface::mk_delete_cmd()
{
    return (new DeleteCmd(m_hdl));
}

TapInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                       const std::string &name,
                                       Route::prefix_t &prefix):
    RpcCmd(item),
    m_name(name),
    m_prefix(prefix)
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
