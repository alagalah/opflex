/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL2Interface.hpp"

using namespace VPP;

L2Interface::BindCmd::BindCmd(HW::Item<bool> &item,
                              const handle_t &itf,
                              const handle_t &bd,
                              bool is_bvi):
    CmdT(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Interface::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Interface::BindCmd::exec()
{
    // finally... call VPP
}
std::string L2Interface::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-interface-BD-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd.to_string();

    return (s.str());
}

L2Interface::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                                  const handle_t &itf,
                                  const handle_t &bd,
                                  bool is_bvi):
    CmdT(item),
    m_itf(itf),
    m_bd(bd),
    m_is_bvi(is_bvi)
{
}

bool L2Interface::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_bd == other.m_bd) &&
            (m_is_bvi == other.m_is_bvi));
}

rc_t L2Interface::UnbindCmd::exec()
{
    // finally... call VPP
}
std::string L2Interface::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L2-interface-BD-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " bd:" << m_bd.to_string();

    return (s.str());
}
