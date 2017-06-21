/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppL3Config.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
InstDB<std::pair<Interface::key_type, Route::prefix_t>, L3Config> L3Config::m_db;

/**
 * Construct a new object matching the desried state
 */
L3Config::L3Config(const Interface &itf,
                   const Route::prefix_t &pfx):
    m_itf(Interface::find(itf)),
    m_pfx(pfx),
    m_binding(0)
{
}

L3Config::L3Config(const SubInterface &itf,
                   const Route::prefix_t &pfx):
    m_itf(SubInterface::find(itf)),
    m_pfx(pfx),
    m_binding(0)
{
}

L3Config::L3Config(const L3Config& o):
    m_itf(o.m_itf),
    m_pfx(o.m_pfx),
    m_binding(0)
{
}

L3Config::~L3Config()
{
    sweep();

    // not in the DB anymore.
    m_db.release(make_pair(m_itf->key(), m_pfx), this);
}

void L3Config::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding, m_itf->handle(), m_pfx));
    }
    HW::write();
}


std::string L3Config::to_string() const
{
    std::ostringstream s;
    s << "L3-config: " << m_itf->to_string()
      << " prefix:" << m_pfx.to_string()
      << m_binding.to_string();

    return (s.str());
}

void L3Config::update(const L3Config &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_binding.rc())
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_pfx));
    }
}

std::shared_ptr<L3Config> L3Config::find_or_add(const L3Config &temp)
{
    return (m_db.find_or_add(make_pair(temp.m_itf->key(), temp.m_pfx), temp));
}
