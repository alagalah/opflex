/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "VppCmd.hpp"
#include "VppTapInterface.hpp"

extern "C"
{
    #include "vpe.api.vapi.h"
}

using namespace VPP;

TapInterface::EventHandler TapInterface::m_evh;

/**
 * Construct a new object matching the desried state
 */
TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix):
    Interface(name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address_t::ZERO)
{
}

TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix,
                           const l2_address_t &l2_address):
    Interface(name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address)
{
}

TapInterface::TapInterface(const std::string &name,
                           admin_state_t state,
                           const l2_address_t &l2_address):
    Interface(name, type_t::TAP, state),
    m_prefix(Route::prefix_t::ZERO),
    m_l2_address(l2_address)
{
}

TapInterface::TapInterface(const handle_t &hdl,
                           const std::string &name,
                           admin_state_t state,
                           Route::prefix_t prefix):
    Interface(hdl, l2_address_t::ZERO, name, type_t::TAP, state),
    m_prefix(prefix),
    m_l2_address(l2_address_t::ZERO)
{
}

TapInterface::~TapInterface()
{
    sweep();
    release();
}

TapInterface::TapInterface(const TapInterface& o):
    Interface(o),
    m_prefix(o.m_prefix),
    m_l2_address(o.m_l2_address)
{
}

std::queue<Cmd*> &  TapInterface::mk_create_cmd(std::queue<Cmd*> &q)
{
    q.push(new CreateCmd(m_hdl, name(), m_prefix, m_l2_address));

    return (q);
}

std::queue<Cmd*> &  TapInterface::mk_delete_cmd(std::queue<Cmd*> &q)
{
    q.push(new DeleteCmd(m_hdl));

    return (q);
}

std::shared_ptr<TapInterface> TapInterface::singular() const
{
    return std::dynamic_pointer_cast<TapInterface>(singular_i());
}

std::shared_ptr<Interface> TapInterface::singular_i() const
{
    return m_db.find_or_add(name(), *this);
}

void TapInterface::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    /*
     * dump VPP current states
     */
    TapInterface::DumpCmd::details_type *data;
    std::shared_ptr<TapInterface::DumpCmd> cmd(new TapInterface::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    while (data = cmd->pop())
    {
        std::string name = reinterpret_cast<const char*>(data->dev_name);

        TapInterface itf(name,
                         Interface::admin_state_t::UP,
                         Route::prefix_t::ZERO);

        LOG(ovsagent::DEBUG) << "tap-dump: " << itf.to_string();

        /*
         * Write each of the discovered interfaces into the OM,
         * but disable the HW Command q whilst we do, so that no
         * commands are sent to VPP
         */
        VPP::OM::commit(key, itf);

        free(data);
    }
}

TapInterface::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"tap"}, "TapInterfaces", this);
}

void TapInterface::EventHandler::handle_replay()
{
    m_db.replay();
}

dependency_t TapInterface::EventHandler::order() const
{
    return (dependency_t::INTERFACE);
}

void TapInterface::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
