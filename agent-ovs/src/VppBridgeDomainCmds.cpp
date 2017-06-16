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

#include "VppBridgeDomain.hpp"
#include "VppCmd.hpp"

using namespace VPP;

BridgeDomain::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                   const std::string &name):
    CmdT<HW::Item<handle_t>>(item),
    m_name(name)
{
}

bool BridgeDomain::CreateCmd::operator==(const CreateCmd& other) const
{
    return (m_name == other.m_name);
}

rc_t BridgeDomain::CreateCmd::exec()
{
    // finally... call VPP
}
std::string BridgeDomain::CreateCmd::to_string()
{
    std::ostringstream s;
    s << "bridge-domain-create: " << m_hw_item.to_string();

    return (s.str());
}

BridgeDomain::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item):
    CmdT<HW::Item<handle_t>>(item)
{
}

bool BridgeDomain::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

rc_t BridgeDomain::DeleteCmd::exec()
{
    // finally... call VPP
}
std::string BridgeDomain::DeleteCmd::to_string()
{
    std::ostringstream s;
    s << "bridge-domain-delete: " << m_hw_item.to_string();

    return (s.str());
}
