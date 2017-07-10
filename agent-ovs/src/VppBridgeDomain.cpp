/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppBridgeDomain.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
SingularDB<uint32_t, BridgeDomain> BridgeDomain::m_db;

/**
 * Construct a new object matching the desried state
 */
BridgeDomain::BridgeDomain(uint32_t id):
    m_id(id)
{
}

BridgeDomain::BridgeDomain(const BridgeDomain& o):
    m_id(o.m_id)
{
}

uint32_t BridgeDomain::id() const
{
    return (m_id.data());
}

void BridgeDomain::sweep()
{
    if (rc_t::OK == m_id.rc())
    {
        HW::enqueue(new DeleteCmd(m_id));
    }
    HW::write();
}

BridgeDomain::~BridgeDomain()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_id.data(), this);
}

std::string BridgeDomain::to_string() const
{
    std::ostringstream s;
    s << "bridge-domain:["
      << m_id.to_string()
      << "]";

    return (s.str());
}

void BridgeDomain::update(const BridgeDomain &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_id.rc())
    {
        HW::enqueue(new CreateCmd(m_id));
    }
}

std::shared_ptr<BridgeDomain> BridgeDomain::find_or_add(const BridgeDomain &temp)
{
    return (m_db.find_or_add(temp.m_id.data(), temp));
}

std::shared_ptr<BridgeDomain> BridgeDomain::singular() const
{
    return find_or_add(*this);
}

void BridgeDomain::dump(std::ostream &os)
{
    m_db.dump(os);
}
