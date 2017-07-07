/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppDhcpConfig.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all DHCP configs
 */
InstDB<Interface::key_type, DhcpConfig> DhcpConfig::m_db;

/**
 * Construct a new object matching the desried state
 */
DhcpConfig::DhcpConfig(const Interface &itf,
                       const std::string &hostname):
    m_itf(itf.instance()),
    m_hostname(hostname),
    m_binding(0)
{
}

DhcpConfig::DhcpConfig(const DhcpConfig& o):
    m_itf(o.m_itf),
    m_hostname(o.m_hostname),
    m_binding(0)
{
}

DhcpConfig::~DhcpConfig()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->key(), this);
}

void DhcpConfig::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding, m_itf->handle(), m_hostname));
    }
    HW::write();
}


std::string DhcpConfig::to_string() const
{
    std::ostringstream s;
    s << "Dhcp-config: " << m_itf->to_string()
      << " hostname:" << m_hostname
      << m_binding.to_string();

    return (s.str());
}

void DhcpConfig::update(const DhcpConfig &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (!m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_hostname));
    }
}

std::shared_ptr<DhcpConfig> DhcpConfig::find_or_add(const DhcpConfig &temp)
{
    return (m_db.find_or_add(temp.m_itf->key(), temp));
}

std::shared_ptr<DhcpConfig> DhcpConfig::instance() const
{
    return find_or_add(*this);
}

DhcpConfig::EventListener::EventListener():
    m_status(rc_t::NOOP)
{
}

HW::Item<bool> &DhcpConfig::EventListener::status()
{
    return (m_status);
}
