/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppRouteDomain.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
InstDB<const std::string, RouteDomain> RouteDomain::m_db;

/**
 * Instantiate the ID generator
 */
Route::table_id_t RouteDomain::m_id_generator;

/**
 * Construct a new object matching the desried state
 */
RouteDomain::RouteDomain(const std::string &name):
    m_name(name),
    m_table_id(Route::DEFAULT_TABLE)
{
}

RouteDomain::RouteDomain(const RouteDomain& o):
    m_name(o.m_name),
    m_table_id(o.m_table_id)
{
}

void RouteDomain::bless()
{
    Object::bless();

    /*
     * assign a unique-ID for this RouteDomain for VPP
     */
    m_table_id = ++m_id_generator;
}

Route::table_id_t RouteDomain::table_id() const
{
    assert(is_blessed());

    return (m_table_id);
}

void RouteDomain::sweep()
{
}

RouteDomain::~RouteDomain()
{
    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string RouteDomain::to_string() const
{
    std::ostringstream s;
    s << "route-domain: " << m_name
      << " ID:" << m_table_id;

    return (s.str());
}

void RouteDomain::update(const RouteDomain &desired)
{
    /*
     * No HW configuration associated with a route Domain
     */
}

std::shared_ptr<RouteDomain> RouteDomain::find_or_add(const RouteDomain &temp)
{
    return (m_db.find_or_add(temp.m_name, temp));
}

std::shared_ptr<RouteDomain> RouteDomain::find(const RouteDomain &temp)
{
    return (m_db.find(temp.m_name));
}
