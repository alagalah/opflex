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
InstDB<const std::string, BridgeDomain> BridgeDomain::m_db;

/**
 * Construct a new object matching the desried state
 */
BridgeDomain::BridgeDomain(const std::string &name):
    m_name(name),
    m_hdl(handle_t::INVALID)
{
}

BridgeDomain::BridgeDomain(const BridgeDomain& o):
    m_name(o.m_name),
    m_hdl(handle_t::INVALID)
{
}

const handle_t & BridgeDomain::handle() const
{
    return (m_hdl.data());
}

void BridgeDomain::sweep()
{
    if (m_hdl)
    {
        HW::enqueue(new DeleteCmd(m_hdl));
    }
    HW::write();
}

BridgeDomain::~BridgeDomain()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string BridgeDomain::to_string() const
{
    std::ostringstream s;
    s << "ridge-domain: " << m_name
      << " hdl:" << m_hdl.to_string();

    return (s.str());
}

void BridgeDomain::update(const BridgeDomain &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_hdl.rc())
    {
        HW::enqueue(new CreateCmd(m_hdl, m_name));
    }
}

std::shared_ptr<BridgeDomain> BridgeDomain::find_or_add(const BridgeDomain &temp)
{
    return (m_db.find_or_add(temp.m_name, temp));
}

std::shared_ptr<BridgeDomain> BridgeDomain::find(const BridgeDomain &temp)
{
    return (m_db.find(temp.m_name, temp));
}
