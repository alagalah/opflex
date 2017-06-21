/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppSubInterface.hpp"

using namespace VPP;

/**
 * A DB of all the sub-interfaces, key on the name
 */
InstDB<const std::string, SubInterface> SubInterface::m_db;

/**
 * Construct a new object matching the desried state
 */
SubInterface::SubInterface(const Interface &parent,
                           admin_state_t state,
                           vlan_id_t vlan):
    Interface(mk_name(parent, vlan), parent.type(), state),
    m_parent(Interface::find(parent)),
    m_vlan(vlan)
{
}
    
SubInterface::~SubInterface()
{
    sweep();

    // not in the DB anymore.
    m_db.release(name(), this);
}

SubInterface::SubInterface(const SubInterface& o):
    Interface(o),
    m_parent(o.m_parent),
    m_vlan(o.m_vlan)
{
}

std::string SubInterface::mk_name(const Interface &parent,
                                  vlan_id_t vlan)
{
    return (parent.name() + "." + std::to_string(vlan));
}

std::shared_ptr<SubInterface> SubInterface::find_or_add(const SubInterface &temp)
{
    return (m_db.find_or_add(temp.name(), temp));
}

std::shared_ptr<SubInterface> SubInterface::find(const SubInterface &temp)
{
    return (m_db.find(temp.name()));
}

Cmd* SubInterface::mk_create_cmd()
{
    return (new CreateCmd(m_hdl, m_parent->handle(), m_vlan));
}

Cmd* SubInterface::mk_delete_cmd()
{
    return (new DeleteCmd(m_hdl));
}
