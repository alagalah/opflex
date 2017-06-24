/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

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

const handle_t handle_t::INVALID(~0);

handle_t::handle_t(int value):
    m_value(value)
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
