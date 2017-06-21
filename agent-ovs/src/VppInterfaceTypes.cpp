/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>

#include "VppInterface.hpp"

using namespace VPP;

/*
 * constants and enums
 */
const Interface::type_t Interface::type_t::UNKNWON(0, "nuknown");
const Interface::type_t Interface::type_t::VHOST(1, "vhost");
const Interface::type_t Interface::type_t::BVI(2, "BVI");
const Interface::type_t Interface::type_t::ETHERNET(3, "Ehternet");
const Interface::type_t Interface::type_t::VXLAN(4, "VXLAN");
const Interface::type_t Interface::type_t::AFPACKET(5, "AFPACKET");

const Interface::admin_state_t Interface::admin_state_t::DOWN(0, "down");
const Interface::admin_state_t Interface::admin_state_t::UP(1, "up");

Interface::type_t Interface::type_t::from_string(const std::string &str)
{
    if (str.find("Ethernet") != std::string::npos)
    {
        return Interface::type_t::ETHERNET;
    }

    return Interface::type_t::UNKNWON;
}

Interface::type_t::type_t(int v, const std::string &s):
    Enum<Interface::type_t>(v, s)
{
}

Interface::admin_state_t::admin_state_t(int v, const std::string &s):
    Enum<Interface::admin_state_t>(v, s)
{
}

Interface::admin_state_t Interface::admin_state_t::from_int(uint8_t v)
{
    if (0 == v)
    {
        return (Interface::admin_state_t::DOWN);
    }
    return (Interface::admin_state_t::UP);
}

