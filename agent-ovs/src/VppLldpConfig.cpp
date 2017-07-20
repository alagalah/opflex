/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppLldpConfig.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all LLDP configs
 */
SingularDB<Interface::key_type, LldpConfig> LldpConfig::m_db;

LldpConfig::EventHandler LldpConfig::m_evh;

LldpConfig::LldpConfig(const Interface &itf,
                       const std::string &port_desc):
    m_itf(itf.singular()),
    m_port_desc(port_desc),
    m_binding(0)
{
}

LldpConfig::LldpConfig(const LldpConfig& o):
    m_itf(o.m_itf),
    m_port_desc(o.m_port_desc),
    m_binding(0)
{
}

LldpConfig::~LldpConfig()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->key(), this);
}

void LldpConfig::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding, m_itf->handle()));
    }
    HW::write();
}

void LldpConfig::dump(std::ostream &os)
{
    m_db.dump(os);
}

void LldpConfig::replay()
{
    if (m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_port_desc));
    }
}

std::string LldpConfig::to_string() const
{
    std::ostringstream s;
    s << "Lldp-config: " << m_itf->to_string()
      << " port_desc:" << m_port_desc
      << " " << m_binding.to_string();

    return (s.str());
}

void LldpConfig::update(const LldpConfig &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (!m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_port_desc));
    }
}

std::shared_ptr<LldpConfig> LldpConfig::find_or_add(const LldpConfig &temp)
{
    return (m_db.find_or_add(temp.m_itf->key(), temp));
}

std::shared_ptr<LldpConfig> LldpConfig::singular() const
{
    return find_or_add(*this);
}

LldpConfig::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"lldp"}, "LLDP configurations", this);
}

void LldpConfig::EventHandler::handle_replay()
{
    m_db.replay();
}

void LldpConfig::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    // FIXME
}

dependency_t LldpConfig::EventHandler::order() const
{
    return (dependency_t::BINDING);
}

void LldpConfig::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
