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
 * A DB of al the sub-interfaces, key on the name
 */
InstDB<const std::string, Interface> Interface::m_db;

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
    m_table_id(Route::DEFAULT_TABLE)
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
    m_table_id(m_rd->table_id())
{
}

Interface::Interface(const Interface& o):
    m_name(o.m_name),
    m_state(o.m_state),
    m_type(o.m_type),
    m_table_id(o.m_table_id),
    m_rd(o.m_rd),
    m_hdl(handle_t::INVALID)
{
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

    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string Interface::to_string() const
{
    std::ostringstream s;
    s << "interface: " << m_name
      << " type:" << m_type.to_string()
      << " hdl:" << m_hdl.to_string()
      << " state:" << m_state.to_string();

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
    return (new CreateCmd(m_hdl, m_name, m_type));
}

Cmd* Interface::mk_delete_cmd()
{
    return (new DeleteCmd(m_hdl, m_type));
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

std::shared_ptr<Interface> Interface::find_or_add(const Interface &temp)
{
    return (m_db.find_or_add(temp.m_name, temp));
}

std::shared_ptr<Interface> Interface::find(const Interface &temp)
{
    return (m_db.find(temp.m_name, temp));
}
