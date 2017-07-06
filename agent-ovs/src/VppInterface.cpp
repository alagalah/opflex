/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppInterface.hpp"
#include "VppRoute.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of all the interfaces, key on the name
 */
InstDB<const std::string, Interface> Interface::m_db;

/**
 * A DB of all the interfaces, key on VPP's handle
 */
std::map<handle_t, std::weak_ptr<Interface>> Interface::m_hdl_db;

/**
 * Construct a new object matching the desried state
 */
Interface::Interface(const std::string &name,
                     Interface::type_t itf_type,
                     Interface::admin_state_t itf_state):
    m_name(name),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_table_id(Route::DEFAULT_TABLE),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const vapi_payload_sw_interface_details &vd):
    m_name(reinterpret_cast<const char*>(vd.interface_name)),
    m_state(Interface::admin_state_t::from_int(vd.link_up_down),
            rc_t::NOOP),
    m_type(Interface::type_t::from_string(m_name)),
    m_hdl(handle_t(vd.sw_if_index), rc_t::NOOP),
    m_table_id(Route::DEFAULT_TABLE),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const std::string &name,
                     Interface::type_t itf_type,
                     Interface::admin_state_t itf_state,
                     const RouteDomain &rd):
    m_name(name),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_rd(RouteDomain::find(rd)),
    m_table_id(m_rd->table_id()),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const Interface& o):
    m_name(o.m_name),
    m_state(o.m_state),
    m_type(o.m_type),
    m_table_id(o.m_table_id),
    m_rd(o.m_rd),
    m_hdl(handle_t::INVALID),
    m_oper(o.m_oper)
{
}

Interface::EventListener::EventListener():
    m_status(rc_t::NOOP)
{
}

HW::Item<bool> &Interface::EventListener::status()
{
    return (m_status);
}

/**
 * Return the interface type
 */
const Interface::type_t & Interface::type() const
{
    return (m_type);
}

const handle_t & Interface::handle() const
{
    return (m_hdl.data());
}

void Interface::sweep()
{
    if (m_table_id)
    {
        m_table_id.data() = Route::DEFAULT_TABLE;
        HW::enqueue(new SetTableCmd(m_table_id, m_hdl));
    }

    // If the interface is up, bring it down
    if (m_state &&
        Interface::admin_state_t::UP == m_state.data())
    {
        m_state.data() = Interface::admin_state_t::DOWN;
        HW::enqueue(new StateChangeCmd(m_state, m_hdl));
    }
    if (m_hdl)
    {
        HW::enqueue(mk_delete_cmd());
    }
    HW::write();
}

Interface::~Interface()
{
    sweep();
    release();
}

void Interface::release()
{
    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string Interface::to_string() const
{
    std::ostringstream s;
    s << "interface: " << m_name
      << " type:" << m_type.to_string()
      << " hdl:" << m_hdl.to_string()
      << " admin-state:" << m_state.to_string()
      << " oper-state:" << m_oper.to_string();

    return (s.str());
}

const std::string &Interface::name() const
{
    return (m_name);
}

const Interface::key_type &Interface::key() const
{
    return (name());
}

Cmd* Interface::mk_create_cmd()
{
    if ((type_t::LOOPBACK == m_type) ||
        (type_t::BVI == m_type))
    {
        return (new LoopbackCreateCmd(m_hdl, m_name));
    }
    else if (type_t::AFPACKET == m_type)
    {
        return (new AFPacketCreateCmd(m_hdl, m_name));
    }
    else if (type_t::TAP == m_type)
    {
        return (new TapCreateCmd(m_hdl, m_name));
    }

    return (nullptr);
}

Cmd* Interface::mk_delete_cmd()
{
    if ((type_t::LOOPBACK == m_type) ||
        (type_t::BVI == m_type))
    {
        return (new LoopbackDeleteCmd(m_hdl));
    }
    else if (type_t::AFPACKET == m_type)
    {
        return (new AFPacketDeleteCmd(m_hdl, m_name));
    }
    else if (type_t::TAP == m_type)
    {
        return (new TapDeleteCmd(m_hdl));
    }
}


void Interface::update(const Interface &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_hdl.rc())
    {
        HW::enqueue(mk_create_cmd());
    }

    /*
     * change the interface state to that which is deisred
     */
    if (m_state.update(desired.m_state))
    {
        HW::enqueue(new StateChangeCmd(m_state, m_hdl));
    }

    /*
     * If the interface is mapped into a route domain, set VPP's
     * table ID
     */
    if (!m_table_id && m_rd)
    {
        HW::enqueue(new SetTableCmd(m_table_id, m_hdl));
    }        
}

void Interface::set(const oper_state_t &state)
{
    m_oper = state;
}

std::shared_ptr<Interface> Interface::find_or_add(const Interface &temp)
{
    return (m_db.find_or_add(temp.m_name, temp));
}

void Interface::insert(const Interface &temp, std::shared_ptr<Interface> sp)
{
    return (m_db.add(temp.m_name, sp));
}

std::shared_ptr<Interface> Interface::find(const Interface &temp)
{
    return (m_db.find(temp.m_name));
}

std::shared_ptr<Interface> Interface::find(const std::string &name)
{
    return (m_db.find(name));
}

std::shared_ptr<Interface> Interface::find(const handle_t &handle)
{
    return (m_hdl_db[handle].lock());
}

void Interface::add(const handle_t &handle,
                    std::shared_ptr<Interface> sp)
{
    m_hdl_db[handle] = sp;
}

void Interface::remove(const handle_t &handle)
{
    m_hdl_db.erase(handle);
}
