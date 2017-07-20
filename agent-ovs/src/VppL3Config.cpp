/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppL3Config.hpp"
#include "VppCmd.hpp"

using namespace VPP;

SingularDB<std::pair<Interface::key_type, Route::prefix_t>, L3Config> L3Config::m_db;

L3Config::EventHandler L3Config::m_evh;

/**
 * Construct a new object matching the desried state
 */
L3Config::L3Config(const Interface &itf,
                   const Route::prefix_t &pfx):
    m_itf(itf.singular()),
    m_pfx(pfx),
    m_binding(true)
{
}

L3Config::L3Config(const L3Config& o):
    m_itf(o.m_itf),
    m_pfx(o.m_pfx),
    m_binding(true)
{
}

L3Config::~L3Config()
{
    sweep();

    // not in the DB anymore.
    m_db.release(make_pair(m_itf->key(), m_pfx), this);
}

void L3Config::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding, m_itf->handle(), m_pfx));
    }
    HW::write();
}

void L3Config::replay()
{
    if (m_binding)
    {
      HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_pfx));
    }
}

const Route::prefix_t& L3Config::prefix() const
{
    return (m_pfx);
}


std::string L3Config::to_string() const
{
    std::ostringstream s;
    s << "L3-config:[" << m_itf->to_string()
      << " prefix:" << m_pfx.to_string()
      << " "
      << m_binding.to_string()
      << "]";

    return (s.str());
}

void L3Config::update(const L3Config &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (!m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle(), m_pfx));
    }
}

std::shared_ptr<L3Config> L3Config::find_or_add(const L3Config &temp)
{
    return (m_db.find_or_add(make_pair(temp.m_itf->key(), temp.m_pfx), temp));
}

std::shared_ptr<L3Config> L3Config::singular() const
{
    return find_or_add(*this);
}

void L3Config::dump(std::ostream &os)
{
    m_db.dump(os);
}

std::ostream& VPP::operator<<(std::ostream &os, const L3Config::key_type_t &key)
{
    os << "["
       << key.first
       << ", "
       << key.second
       << "]";

    return (os);
}

std::deque<std::shared_ptr<L3Config>> L3Config::find(const Interface &i)
{
    /*
     * Loop throught the entire map looking for matching interface.
     * not the most efficient algorithm, but it will do for now. The
     * number of L3 configs is low and this is only called during bootup
     */
    std::deque<std::shared_ptr<L3Config>> l3s;

    auto it = m_db.cbegin();

    while (it != m_db.cend())
    {
        /*
         * The key in the DB is a pair of the interface's name and prefix.
         * If the keys match, save the L3-config
         */
        auto key = it->first;

        if (i.key() == key.first)
        {
            l3s.push_back(it->second.lock());
        }

        ++it;
    }

    return (l3s);
}

L3Config::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"l3"}, "L3 Bindings", this);
}

void L3Config::EventHandler::handle_replay()
{
    m_db.replay();
}

void L3Config::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    /**
     * This is done while populating the interfaces
     */
}

dependency_t L3Config::EventHandler::order() const
{
    return (dependency_t::BINDING);
}

void L3Config::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
