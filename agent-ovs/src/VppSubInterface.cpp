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
 * Construct a new object matching the desried state
 */
SubInterface::SubInterface(const Interface &parent,
                           admin_state_t state,
                           vlan_id_t vlan):
    Interface(mk_name(parent, vlan), parent.type(), state),
    m_parent(parent.instance()),
    m_vlan(vlan)
{
}

SubInterface::SubInterface(const handle_t &handle,
                           const Interface &parent,
                           admin_state_t state,
                           vlan_id_t vlan):
    Interface(handle, mk_name(parent, vlan), parent.type(), state),
    m_parent(parent.instance()),
    m_vlan(vlan)
{
}
    
SubInterface::~SubInterface()
{
    sweep();
    release();
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

std::queue<Cmd*> & SubInterface::mk_create_cmd(std::queue<Cmd*> &q)
{
    q.push(new CreateCmd(m_hdl, name(), m_parent->handle(), m_vlan));

    return (q);
}

std::queue<Cmd*> & SubInterface::mk_delete_cmd(std::queue<Cmd*> &q)
{
    q.push(new DeleteCmd(m_hdl));

    return (q);
}

std::shared_ptr<SubInterface> SubInterface::instance() const
{
    return std::dynamic_pointer_cast<SubInterface>(instance_i());
}

std::shared_ptr<Interface> SubInterface::instance_i() const
{
    return m_db.find_or_add(name(), *this);
}
