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
InstDB<Route::table_id_t, RouteDomain> RouteDomain::m_db;

/**
 * Construct a new object matching the desried state
 */
RouteDomain::RouteDomain(Route::table_id_t id):
    m_table_id(id)
{
}

RouteDomain::RouteDomain(const RouteDomain& o):
    m_table_id(o.m_table_id)
{
}

Route::table_id_t RouteDomain::table_id() const
{
    return (m_table_id);
}

void RouteDomain::sweep()
{
}

RouteDomain::~RouteDomain()
{
    // not in the DB anymore.
    m_db.release(m_table_id, this);
}

std::string RouteDomain::to_string() const
{
    std::ostringstream s;
    s << "route-domain:["
      << m_table_id
      << "]";

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
    return (m_db.find_or_add(temp.m_table_id, temp));
}

std::shared_ptr<RouteDomain> RouteDomain::instance() const
{
    return find_or_add(*this);
}

void RouteDomain::dump(std::ostream &os)
{
    m_db.dump(os);
}
