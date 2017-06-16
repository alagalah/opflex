/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppSubnet.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
InstDB<const Route::prefix_t, Subnet> Subnet::m_db;

/**
 * Construct a new object matching the desried state
 */
Subnet::Subnet(const Route::prefix_t &p):
    m_prefix(p)
{
}

Subnet::Subnet(const Subnet& o):
    m_prefix(o.m_prefix)
{
}

void Subnet::sweep()
{
}

Subnet::~Subnet()
{
    // not in the DB anymore.
    m_db.release(m_prefix, this);
}

std::string Subnet::to_string()
{
    std::ostringstream s;
    s << "subnet: " << m_prefix.to_string();

    return (s.str());
}

void Subnet::update(const Subnet &desired)
{
}

std::shared_ptr<Subnet> Subnet::find_or_add(const Subnet &temp)
{
    return (m_db.find_or_add(temp.m_prefix, temp));
}

std::shared_ptr<Subnet> Subnet::find(const Subnet &temp)
{
    return (m_db.find(temp.m_prefix, temp));
}
