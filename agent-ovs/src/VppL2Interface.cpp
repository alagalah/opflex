/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppL2Interface.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
InstDB<const handle_t, L2Interface> L2Interface::m_db;

/**
 * Construct a new object matching the desried state
 */
L2Interface::L2Interface(const Interface &itf,
                         const BridgeDomain &bd):
    m_itf(Interface::find(itf)),
    m_bd(BridgeDomain::find(bd)),
    m_binding(0)
{
}

L2Interface::L2Interface(const L2Interface& o):
    m_itf(o.m_itf),
    m_bd(o.m_bd),
    m_binding(0)
{
}

void L2Interface::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding,
                                  m_itf->handle(),
                                  m_bd->handle(),
                                  Interface::type_t::BVI == m_itf->type()));
    }
}

L2Interface::~L2Interface()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->handle(), this);
}

std::string L2Interface::to_string()
{
    std::ostringstream s;
    s << "L2-interface: " << m_itf->to_string()
      << " BD:" << m_bd->to_string()
      << m_binding.to_string();

    return (s.str());
}

void L2Interface::update(const L2Interface &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_binding.rc())
    {
        HW::enqueue(new BindCmd(m_binding,
                                  m_itf->handle(),
                                  m_bd->handle(),
                                  Interface::type_t::BVI == m_itf->type()));
    }
}

std::shared_ptr<L2Interface> L2Interface::find_or_add(const L2Interface &temp)
{
    return (m_db.find_or_add(temp.m_itf->handle(), temp));
}
