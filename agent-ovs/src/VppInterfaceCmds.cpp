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

#include "VppInterface.hpp"
#include "VppCmd.hpp"


using namespace VPP;

Interface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                  const std::string &name,
                                  Interface::type_t type):
    CmdT<HW::Item<handle_t>>(item),
    m_name(name),
    m_type(type)
{
}

Interface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                  Interface::type_t type):
    CmdT<HW::Item<handle_t>>(item),
    m_name(Interface::NAMELESS),
    m_type(type)
{
}

bool Interface::CreateCmd::operator==(const CreateCmd& other) const
{
    return ((m_type == other.m_type) &&
            (m_name == other.m_name));
}

rc_t Interface::CreateCmd::exec()
{
    // finally... call VPP
}
std::string Interface::CreateCmd::to_string()
{
    std::ostringstream s;
    s << "itf-create: " << m_hw_item.to_string()
      << " name:" << m_name
      << " type:" << m_type.to_string();
    return (s.str());
}

Interface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item,
                                Interface::type_t type):
    CmdT<HW::Item<handle_t>>(item),
    m_type(type)
{
}

bool Interface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return ((m_type == other.m_type) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::DeleteCmd::exec()
{
    // finally... call VPP
}
std::string Interface::DeleteCmd::to_string()
{
    std::ostringstream s;
    s << "itf-delete: " << m_hw_item.to_string()
      << " type:" << m_type.to_string();
    return (s.str());
}

Interface::StateChangeCmd::StateChangeCmd(HW::Item<Interface::admin_state_t> &state,
                                            const HW::Item<handle_t> &hdl):
    CmdT<HW::Item<Interface::admin_state_t> >(state),
    m_hdl(hdl)
{
}

bool Interface::StateChangeCmd::operator==(const StateChangeCmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::StateChangeCmd::exec()
{
    // finally... call VPP
}

std::string Interface::StateChangeCmd::to_string()
{
    std::ostringstream s;
    s << "itf-state-change: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}

Interface::PrefixAddCmd::PrefixAddCmd(HW::Item<Route::prefix_t> &item,
                                      const HW::Item<handle_t> &hdl):
    CmdT(item),
    m_hdl(hdl)
{
}

bool Interface::PrefixAddCmd::operator==(const PrefixAddCmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::PrefixAddCmd::exec()
{
    // finally... call VPP
}

std::string Interface::PrefixAddCmd::to_string()
{
    std::ostringstream s;
    s << "itf-prefix-add: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}

Interface::PrefixDelCmd::PrefixDelCmd(HW::Item<Route::prefix_t> &item,
                                      const HW::Item<handle_t> &hdl):
    CmdT(item),
    m_hdl(hdl)
{
}

bool Interface::PrefixDelCmd::operator==(const PrefixDelCmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::PrefixDelCmd::exec()
{
    // finally... call VPP
}

std::string Interface::PrefixDelCmd::to_string()
{
    std::ostringstream s;
    s << "itf-prefix-del: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}
