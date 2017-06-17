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
SubInterface::SubInterface(const std::string &name,
                           const std::shared_ptr<Interface> parent,
                           admin_state_t state,
                           vlan_id_t vlan):
    Interface(name, parent->type(), state),
    m_parent(parent),
    m_vlan(vlan)
{
}
    

SubInterface::SubInterface(const SubInterface& o):
    Interface(o),
    m_parent(o.m_parent),
    m_vlan(o.m_vlan)
{
}

