/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppL2Config.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
SingularDB<const handle_t, L2Config> L2Config::m_db;

/**
 * Construct a new object matching the desried state
 */
L2Config::L2Config(const Interface &itf,
                   const BridgeDomain &bd):
    m_itf(itf.singular()),
    m_bd(bd.singular()),
    m_binding(0)
{
}

L2Config::L2Config(const L2Config& o):
    m_itf(o.m_itf),
    m_bd(o.m_bd),
    m_binding(0)
{
}

void L2Config::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding,
                                  m_itf->handle(),
                                  m_bd->id(),
                                  Interface::type_t::BVI == m_itf->type()));
    }
    HW::write();
}

L2Config::~L2Config()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->handle(), this);
}

std::string L2Config::to_string() const
{
    std::ostringstream s;
    s << "L2-config:[" << m_itf->to_string()
      << " " << m_bd->to_string()
      << " " << m_binding.to_string()
      << "]";

    return (s.str());
}

void L2Config::update(const L2Config &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_binding.rc())
    {
        HW::enqueue(new BindCmd(m_binding,
                                m_itf->handle(),
                                m_bd->id(),
                                Interface::type_t::BVI == m_itf->type()));
    }
}

std::shared_ptr<L2Config> L2Config::find_or_add(const L2Config &temp)
{
    return (m_db.find_or_add(temp.m_itf->handle(), temp));
}

std::shared_ptr<L2Config> L2Config::singular() const
{
    return find_or_add(*this);
}

void L2Config::dump(std::ostream &os)
{
    m_db.dump(os);
}
