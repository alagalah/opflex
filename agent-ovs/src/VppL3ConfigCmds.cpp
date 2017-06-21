/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppL3Config.hpp"

using namespace VPP;

L3Config::BindCmd::BindCmd(HW::Item<bool> &item,
                           const handle_t &itf,
                           const Route::prefix_t &pfx):
    CmdT(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Config::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Config::BindCmd::exec()
{
    // finally... call VPP
    return (rc_t::OK);
}

std::string L3Config::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-config-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}

L3Config::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                               const handle_t &itf,
                               const Route::prefix_t &pfx):
    CmdT(item),
    m_itf(itf),
    m_pfx(pfx)
{
}

bool L3Config::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_pfx == other.m_pfx));
}

rc_t L3Config::UnbindCmd::exec()
{
    // finally... call VPP
    return (rc_t::OK);
}

std::string L3Config::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "L3-config-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " pfx:" << m_pfx.to_string();

    return (s.str());
}
