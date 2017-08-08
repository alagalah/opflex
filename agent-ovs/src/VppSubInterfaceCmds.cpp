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

#include "VppSubInterface.hpp"
#include "VppCmd.hpp"

#include <vapi/vpe.api.vapi.hpp>

using namespace VPP;

SubInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                   const std::string &name,
                                   const handle_t &parent,
                                   uint16_t vlan):
    Interface::CreateCmd<vapi::Create_vlan_subif>(item, name),
    m_parent(parent),
    m_vlan(vlan)
{
}

bool SubInterface::CreateCmd::operator==(const CreateCmd& other) const
{
    return ((m_name == other.m_name) &&
            (m_parent == other.m_parent) &&
            (m_vlan == other.m_vlan));
}

rc_t SubInterface::CreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_parent.value();
    payload.vlan_id = m_vlan;

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        Interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}

std::string SubInterface::CreateCmd::to_string() const
{
    std::ostringstream s;
    s << "sub-itf-create: " << m_hw_item.to_string()
      << " parent:" << m_parent
      << " vlan:" << m_vlan;
    return (s.str());
}

SubInterface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item):
    Interface::DeleteCmd<vapi::Delete_subif>(item)
{
}

bool SubInterface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

rc_t SubInterface::DeleteCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hw_item.data().value();

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    Interface::remove(m_hw_item);
    return (rc_t::OK);
}

std::string SubInterface::DeleteCmd::to_string() const
{
    std::ostringstream s;

    s << "sub-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

