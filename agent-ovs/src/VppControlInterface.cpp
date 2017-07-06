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
#include "VppControlInterface.hpp"

extern "C"
{
    #include "vpe.api.vapi.h"
}

using namespace VPP;

/**
 * A DB of all the control-interfaces, key on the name
 */
InstDB<const std::string, ControlInterface> ControlInterface::m_db;

/**
 * Construct a new object matching the desried state
 */
ControlInterface::ControlInterface(const std::string &name,
                                   type_t type,
                                   admin_state_t state,
                                   Route::prefix_t prefix):
    Interface(name, type, state),
    m_interface(Interface(name, type, state)),
    m_prefix(prefix)
{
}

ControlInterface::~ControlInterface()
{
    sweep();

    // not in the DB anymore.
    Interface::release();
    m_db.release(name(), this);
}

ControlInterface::ControlInterface(const ControlInterface& o):
    Interface(o),
    m_interface(o.m_interface),
    m_prefix(o.m_prefix)
{
}
/*
ControlInterface::ControlInterface(const std::string &name,
                  type_t type,
                  admin_state_t state,
                  ip_addr_t ip):
    m_name(name),
    m_state(state),
    m_type(type),
    m_hdl(handle_t::INVALID),
    m_table_id(Route::DEFAULT_TABLE),
    m_oper(oper_state_t::DOWN)
    m_ip(ip)
{
}
*/
std::shared_ptr<ControlInterface> ControlInterface::find_or_add(const ControlInterface &temp)
{
    std::shared_ptr<ControlInterface> sp = m_db.find_or_add(temp.key(), temp);

    Interface::insert(temp, sp);

    return (sp);
}

std::shared_ptr<ControlInterface> ControlInterface::find(const ControlInterface &temp)
{
    return (m_db.find(temp.name()));
}

Cmd* ControlInterface::mk_create_cmd()
{
    return (new CreateCmd(m_hdl, m_interface, m_prefix));
}

Cmd* ControlInterface::mk_delete_cmd()
{
    return (new DeleteCmd(m_hdl));
}

ControlInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                       Interface &interface,
                                       Route::prefix_t &prefix):
    RpcCmd(item),
    m_interface(interface),
    m_prefix(prefix)
{
}

rc_t ControlInterface::CreateCmd::issue(Connection &con)
{
    vapi_msg_tap_connect *req;

    req = vapi_alloc_tap_connect(con.ctx());
    memset(req->payload.tap_name, 0,
                    sizeof(req->payload.tap_name));
    memcpy(req->payload.tap_name, m_interface.name().c_str(),
           std::min(m_interface.name().length(),
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

    vapi_tap_connect(con.ctx(), req,
                          Interface::create_callback<
                          vapi_payload_tap_connect_reply,
                          CreateCmd>,
                          this);
   m_hw_item =  wait();

    return rc_t::OK;
}

std::string ControlInterface::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-intf-create: " << m_hw_item.to_string()
      << " interface:" << m_interface
      << " ip-prefix:" << m_prefix.to_string(); 

    return (s.str());
}

bool ControlInterface::CreateCmd::operator==(const CreateCmd& other) const
{
    return (CreateCmd::operator==(other));
}

ControlInterface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item):
    RpcCmd(item)
{
}

rc_t ControlInterface::DeleteCmd::issue(Connection &con)
{
    // finally... call VPP

    return rc_t::OK;
}
std::string ControlInterface::DeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "control-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

bool ControlInterface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}
