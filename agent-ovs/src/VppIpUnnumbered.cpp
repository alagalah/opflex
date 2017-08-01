/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppIpUnnumbered.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all LLDP configs
 */
SingularDB<IpUnnumbered::key_t, IpUnnumbered> IpUnnumbered::m_db;

IpUnnumbered::EventHandler IpUnnumbered::m_evh;

IpUnnumbered::IpUnnumbered(const Interface &itf,
                           const Interface &l3_itf):
    m_itf(itf.singular()),
    m_l3_itf(l3_itf.singular())
{
}

IpUnnumbered::IpUnnumbered(const IpUnnumbered& o):
    m_itf(o.m_itf),
    m_l3_itf(o.m_l3_itf),
    m_config(o.m_config)
{
}

IpUnnumbered::~IpUnnumbered()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->key(), this);
}

void IpUnnumbered::sweep()
{
    if (m_config)
    {
        HW::enqueue(new UnconfigCmd(m_config, m_itf->handle(), m_l3_itf->handle()));
    }
}

void IpUnnumbered::dump(std::ostream &os)
{
    m_db.dump(os);
}

void IpUnnumbered::replay()
{
    if (m_config)
    {
        HW::enqueue(new ConfigCmd(m_config, m_itf->handle(), m_l3_itf->handle()));
    }
}

std::string IpUnnumbered::to_string() const
{
    std::ostringstream s;
    s << "IP Unnumbered-config:"
      << " itf:" << m_itf->to_string()
      << " l3-itf:" << m_l3_itf->to_string();

    return (s.str());
}

void IpUnnumbered::update(const IpUnnumbered &desired)
{
    if (!m_config)
    {
        HW::enqueue(new ConfigCmd(m_config, m_itf->handle(), m_l3_itf->handle()));
    }
}

std::shared_ptr<IpUnnumbered> IpUnnumbered::find_or_add(const IpUnnumbered &temp)
{
    return (m_db.find_or_add(temp.m_itf->key(), temp));
}

std::shared_ptr<IpUnnumbered> IpUnnumbered::singular() const
{
    return find_or_add(*this);
}

IpUnnumbered::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"ip-un"}, "IP unnumbered configurations", this);
}

void IpUnnumbered::EventHandler::handle_replay()
{
    m_db.replay();
}

void IpUnnumbered::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    // VPP provides no dump for IP unnumbered
}

dependency_t IpUnnumbered::EventHandler::order() const
{
    return (dependency_t::BINDING);
}

void IpUnnumbered::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
