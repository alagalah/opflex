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


using namespace VPP;

SubInterface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                   handle_t parent,
                                   uint16_t vlan):
    CmdT<HW::Item<handle_t>>(item),
    m_parent(parent),
    m_vlan(vlan)
{
}

bool SubInterface::CreateCmd::operator==(const CreateCmd& other) const
{
    return ((m_parent == other.m_parent) &&
            (m_vlan == other.m_vlan));
}

rc_t SubInterface::CreateCmd::exec()
{
    // finally... call VPP
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
    CmdT<HW::Item<handle_t>>(item)
{
}

bool SubInterface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

rc_t SubInterface::DeleteCmd::exec()
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

