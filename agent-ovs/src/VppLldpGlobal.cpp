/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppLldpGlobal.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all LLDP configs
 */
SingularDB<std::string, LldpGlobal> LldpGlobal::m_db;

LldpGlobal::EventHandler LldpGlobal::m_evh;

LldpGlobal::LldpGlobal(const std::string &system_name,
                       uint32_t tx_hold,
                       uint32_t tx_interval):
    m_system_name(system_name),
    m_tx_hold(tx_hold),
    m_tx_interval(tx_interval)
{
}

LldpGlobal::LldpGlobal(const LldpGlobal& o):
    m_system_name(o.m_system_name),
    m_tx_hold(o.m_tx_hold),
    m_tx_interval(o.m_tx_interval)
{
}

LldpGlobal::~LldpGlobal()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_system_name, this);
}

void LldpGlobal::sweep()
{
    // no means to remove this in VPP
}

void LldpGlobal::dump(std::ostream &os)
{
    m_db.dump(os);
}

void LldpGlobal::replay()
{
    if (m_binding)
    {
        HW::enqueue(new ConfigCmd(m_binding, m_system_name, m_tx_hold, m_tx_interval));
    }
}

std::string LldpGlobal::to_string() const
{
    std::ostringstream s;
    s << "LLDP-global:"
      << " system_name:" << m_system_name
      << " tx-hold:" << m_tx_hold
      << " tx-interval:" << m_tx_interval;

    return (s.str());
}

void LldpGlobal::update(const LldpGlobal &desired)
{
    if (!m_binding)
    {
        HW::enqueue(new ConfigCmd(m_binding, m_system_name, m_tx_hold, m_tx_interval));
    }
}

std::shared_ptr<LldpGlobal> LldpGlobal::find_or_add(const LldpGlobal &temp)
{
    return (m_db.find_or_add(temp.m_system_name, temp));
}

std::shared_ptr<LldpGlobal> LldpGlobal::singular() const
{
    return find_or_add(*this);
}

LldpGlobal::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"lldp-global"}, "LLDP global configurations", this);
}

void LldpGlobal::EventHandler::handle_replay()
{
    m_db.replay();
}

void LldpGlobal::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    // FIXME
}

dependency_t LldpGlobal::EventHandler::order() const
{
    return (dependency_t::GLOBAL);
}

void LldpGlobal::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
