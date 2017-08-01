/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppArpProxyBinding.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all LLDP configs
 */
SingularDB<Interface::key_type, ArpProxyBinding> ArpProxyBinding::m_db;

ArpProxyBinding::EventHandler ArpProxyBinding::m_evh;

ArpProxyBinding::ArpProxyBinding(const Interface &itf,
                                 const ArpProxyConfig &proxy_cfg):
    m_itf(itf.singular()),
    m_arp_proxy_cfg(proxy_cfg.singular()),
    m_binding(false)
{
}

ArpProxyBinding::ArpProxyBinding(const ArpProxyBinding& o):
    m_itf(o.m_itf),
    m_arp_proxy_cfg(o.m_arp_proxy_cfg),
    m_binding(false)
{
}

ArpProxyBinding::~ArpProxyBinding()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_itf->key(), this);
}

void ArpProxyBinding::sweep()
{
    if (m_binding)
    {
        HW::enqueue(new UnbindCmd(m_binding, m_itf->handle()));
    }
    HW::write();
}

void ArpProxyBinding::dump(std::ostream &os)
{
    m_db.dump(os);
}

void ArpProxyBinding::replay()
{
    if (m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle()));
    }
}

std::string ArpProxyBinding::to_string() const
{
    std::ostringstream s;
    s << "ArpProxy-binding: " << m_itf->to_string();

    return (s.str());
}

void ArpProxyBinding::update(const ArpProxyBinding &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (!m_binding)
    {
        HW::enqueue(new BindCmd(m_binding, m_itf->handle()));
    }
}

std::shared_ptr<ArpProxyBinding> ArpProxyBinding::find_or_add(const ArpProxyBinding &temp)
{
    return (m_db.find_or_add(temp.m_itf->key(), temp));
}

std::shared_ptr<ArpProxyBinding> ArpProxyBinding::singular() const
{
    return find_or_add(*this);
}

ArpProxyBinding::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"arp-proxy"}, "ARP proxy bindings", this);
}

void ArpProxyBinding::EventHandler::handle_replay()
{
    m_db.replay();
}

void ArpProxyBinding::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    // FIXME
}

dependency_t ArpProxyBinding::EventHandler::order() const
{
    return (dependency_t::BINDING);
}

void ArpProxyBinding::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
