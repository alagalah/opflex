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

extern "C"
{
    #include "vpe.api.vapi.h"
}

using namespace VPP;

SubInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                   const handle_t &parent,
                                   uint16_t vlan):
    RpcCmd(item),
    m_parent(parent),
    m_vlan(vlan)
{
}

bool SubInterface::CreateCmd::operator==(const CreateCmd& other) const
{
    return ((m_parent == other.m_parent) &&
            (m_vlan == other.m_vlan));
}

rc_t SubInterface::CreateCmd::issue(Connection &con)
{
    vapi_msg_create_vlan_subif *req;

    req = vapi_alloc_create_vlan_subif(con.ctx());
    req->payload.sw_if_index = m_parent.value();
    req->payload.vlan_id = m_vlan;

    vapi_create_vlan_subif(con.ctx(), req,
                           Interface::create_callback<
                             vapi_payload_create_vlan_subif_reply,
                             CreateCmd>,
                           this);
    m_hw_item = wait();

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
    RpcCmd(item)
{
}

bool SubInterface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

rc_t SubInterface::DeleteCmd::issue(Connection &con)
{
    // finally... call VPP
    return (rc_t::OK);
}

std::string SubInterface::DeleteCmd::to_string() const
{
    std::ostringstream s;

    s << "sub-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

