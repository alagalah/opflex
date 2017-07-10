/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>

#include "Vpp.hpp"

using namespace VPP;

rc_t::rc_t(int v, const std::string s):
    Enum<rc_t>(v, s)
{
}
rc_t::~rc_t()
{
}

const rc_t &rc_t::from_vpp_retval(int32_t rv)
{
    if (0 == rv)
    {
        return (rc_t::OK);
    }
    if (-68 == rv)
    {
        // interface laready exists
        return (rc_t::OK);
    }

    return (rc_t::INVALID);
}

const rc_t rc_t::UNSET(0, "un-set");
const rc_t rc_t::NOOP(1, "no-op");
const rc_t rc_t::OK(2, "ok");
const rc_t rc_t::INPROGRESS(3, "in-progess");
const rc_t rc_t::INVALID(4, "invalid");
const rc_t rc_t::TIMEOUT(5, "timeout");

const handle_t handle_t::INVALID(~0);

handle_t::handle_t(int value):
    m_value(value)
{
}

handle_t::handle_t():
    m_value(~0)
{
}

std::string handle_t::to_string() const
{
    return (std::to_string(m_value));
}

bool handle_t::operator==(const handle_t &other) const
{
    return (m_value == other.m_value);
}

bool handle_t::operator<(const handle_t &other) const
{
    return (m_value < other.m_value);    
}

uint32_t handle_t::value() const
{
    return (m_value);
}

std::ostream & VPP::operator<<(std::ostream &os, const handle_t &h)
{
    os << h.value();

    return (os);
}

mac_address_t::mac_address_t(uint8_t b[6])
{
    std::copy(b, b+6, std::begin(bytes));
}

mac_address_t::mac_address_t(std::initializer_list<uint8_t> i)
{
    std::copy(i.begin(), i.end(), std::begin(bytes));
}

const mac_address_t mac_address_t::ONE({0xff, 0xff, 0xff, 0xff, 0xff, 0xff});

const mac_address_t mac_address_t::ZERO({ 0x0 });

void mac_address_t::to_bytes(uint8_t *array) const
{
    for (int i = 0; i < 6; i++)
    {
        array[i] = bytes[i];
    }
}

std::string mac_address_t::to_string() const
{
    std::ostringstream s;

    s << "mac:[";
    for (auto byte : bytes)
    {
        s << std::to_string(byte) << ",";
    }
    s << "]";

    return (s.str());
}

bool mac_address_t::operator==(const mac_address_t &mac) const
{
    return (bytes == mac.bytes);
}

std::ostream &VPP::operator<<(std::ostream &os, const mac_address_t &mac)
{
    os << mac.to_string();

    return (os);
}
