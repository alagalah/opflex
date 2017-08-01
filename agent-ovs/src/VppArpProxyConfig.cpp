/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppArpProxyConfig.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all LLDP configs
 */
SingularDB<ArpProxyConfig::key_t, ArpProxyConfig> ArpProxyConfig::m_db;

ArpProxyConfig::EventHandler ArpProxyConfig::m_evh;

ArpProxyConfig::ArpProxyConfig(const boost::asio::ip::address_v4 &low,
                               const boost::asio::ip::address_v4 &high):
    m_high(high),
    m_low(low)
{
}

ArpProxyConfig::ArpProxyConfig(const ArpProxyConfig& o):
    m_low(o.m_low),
    m_high(o.m_high)
{
}

ArpProxyConfig::~ArpProxyConfig()
{
    sweep();

    // not in the DB anymore.
    m_db.release(std::make_pair(m_low, m_high), this);
}

void ArpProxyConfig::sweep()
{
    if (m_config)
    {
        HW::enqueue(new UnconfigCmd(m_config, m_low, m_high));
    }
}

void ArpProxyConfig::dump(std::ostream &os)
{
    m_db.dump(os);
}

void ArpProxyConfig::replay()
{
    if (m_config)
    {
        HW::enqueue(new ConfigCmd(m_config, m_low, m_high));
    }
}

std::string ArpProxyConfig::to_string() const
{
    std::ostringstream s;
    s << "ARP-proxy:"
      << " low:" << m_low.to_string()
      << " high:" << m_high.to_string();

    return (s.str());
}

void ArpProxyConfig::update(const ArpProxyConfig &desired)
{
    if (!m_config)
    {
        HW::enqueue(new ConfigCmd(m_config, m_low, m_high));
    }
}

std::shared_ptr<ArpProxyConfig> ArpProxyConfig::find_or_add(const ArpProxyConfig &temp)
{
    return (m_db.find_or_add(std::make_pair(temp.m_low, temp.m_high), temp));
}

std::shared_ptr<ArpProxyConfig> ArpProxyConfig::singular() const
{
    return find_or_add(*this);
}

ArpProxyConfig::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"arp-proxy"}, "ARP Proxy configurations", this);
}

void ArpProxyConfig::EventHandler::handle_replay()
{
    m_db.replay();
}

void ArpProxyConfig::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    // VPP provides no dump for ARP proxy.
}

dependency_t ArpProxyConfig::EventHandler::order() const
{
    return (dependency_t::GLOBAL);
}

void ArpProxyConfig::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}

std::ostream& VPP::operator<<(std::ostream &os, const ArpProxyConfig::key_t &key)
{
    os << "["
       << key.first
       << ", "
       << key.second
       << "]";

    return (os);
}
