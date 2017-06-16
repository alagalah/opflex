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
#include "VppCmd.hpp"

using namespace VPP;

/**
 * The name of a nameless interface
 */
const std::string Interface::NAMELESS("__no_name__");

/**
 * A DB of al the interfaces, key on the name
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
    m_prefix(Route::prefix_t::ZERO)
{
}
Interface::Interface(const std::string &name,
                     Interface::type_t itf_type,
                     Interface::admin_state_t itf_state,
                     Route::prefix_t prefix):
    m_name(name),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_prefix(prefix)
{
}

Interface::Interface(Interface::type_t itf_type,
                     Interface::admin_state_t itf_state,
                     Route::prefix_t prefix):
    m_name(Interface::NAMELESS),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_prefix(prefix)
{
}

Interface::Interface(const Interface& o):
    m_name(o.m_name),
    m_state(o.m_state),
    m_type(o.m_type),
    m_prefix(o.m_prefix),
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
    if (m_prefix && Route::prefix_t::ZERO != m_prefix.data())
    {
        /*
         * we have a prefix installed in HW
         */
        HW::enqueue(new PrefixDelCmd(m_prefix, m_hdl));
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
        HW::enqueue(new DeleteCmd(m_hdl, m_type));
    }
}

Interface::~Interface()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string Interface::to_string()
{
    std::ostringstream s;
    s << "interface: " << m_name
      << " type:" << m_type.to_string()
      << " hdl:" << m_hdl.to_string()
      << " state:" << m_state.to_string();

    return (s.str());
}

void Interface::update(const Interface &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_hdl.rc())
    {
        HW::enqueue(new CreateCmd(m_hdl, m_name, m_type));
    }

    /*
     * change the interface state to that which is deisred
     */
    if (m_state.update(desired.m_state))
    {
        HW::enqueue(new StateChangeCmd(m_state, m_hdl));
    }

    /*
     * Add an IP address if desired
     */
    if (m_prefix == desired.m_prefix)
    {
        if (!m_prefix && Route::prefix_t::ZERO != m_prefix.data())
        {
            /*
             * we don't have a prefix installed in HW and we want one
             */
            HW::enqueue(new PrefixAddCmd(m_prefix, m_hdl));
        }
    }
    else
    {
        /*
         * prefix dsta has changes
         */
        if (Route::prefix_t::ZERO == m_prefix.data())
        {
            /*
             * we don't have on yet, this is the case of an addition
             * of a new
             */
            m_prefix.update(desired.m_prefix);
            HW::enqueue(new PrefixAddCmd(m_prefix, m_hdl));
        }
        else if (Route::prefix_t::ZERO == desired.m_prefix.data())
        {
            /*
             * We've got one, but no longer want one.
             */
            m_prefix.update(desired.m_prefix);
            HW::enqueue(new PrefixDelCmd(m_prefix, m_hdl));
        }
        else
        {
            /*
             * We've got one, but want a different one.
             */
            HW::enqueue(new PrefixDelCmd(m_prefix, m_hdl));
            m_prefix.update(desired.m_prefix);
            HW::enqueue(new PrefixAddCmd(m_prefix, m_hdl));
        }
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
