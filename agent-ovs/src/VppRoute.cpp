/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "VppRoute.hpp"

using namespace VPP;

/**
 * In VPP 0 represents the default table.
 */
const Route::table_id_t Route::DEFAULT_TABLE = 0;

/**
 * The all Zeros prefix
 */
const Route::prefix_t Route::prefix_t::ZERO("0.0.0.0", 0);

Route::prefix_t::prefix_t(const boost::asio::ip::address &addr,
                          uint8_t len):
    m_addr(addr),
    m_len(len)
{
}

Route::prefix_t::prefix_t(const std::string &s,
                          uint8_t len):
    m_addr(boost::asio::ip::address::from_string(s)),
    m_len(len)
{
}

Route::prefix_t::prefix_t(const prefix_t &o):
    m_addr(o.m_addr),
    m_len(o.m_len)
{
}

Route::prefix_t::prefix_t():
    m_addr(),
    m_len(0)
{
}

Route::prefix_t::~prefix_t()
{
}

Route::prefix_t &Route::prefix_t::operator=(const Route::prefix_t &o)
{
    m_addr = o.m_addr;
    m_len = o.m_len;

    return (*this);
}

const boost::asio::ip::address &Route::prefix_t::address() const
{
    return (m_addr);
}

bool Route::prefix_t::operator<(const Route::prefix_t &o) const
{
    if (m_len == o.m_len)
    {
        return (m_addr < o.m_addr);
    }
    else
    {
        return (m_len < o.m_len);
    }
}

bool Route::prefix_t::operator==(const Route::prefix_t &o) const
{
    return (m_len == o.m_len &&
            m_addr == o.m_addr);
}

bool Route::prefix_t::operator!=(const Route::prefix_t &o) const
{
    return (!(*this == o));
}

std::string Route::prefix_t::to_string() const
{
    std::ostringstream s;

    s << m_addr.to_string()
      << "/" << std::to_string(m_len);

    return (s.str());
}

Route::prefix_t::prefix_t(uint8_t is_ip6,
                          uint8_t *addr,
                          uint8_t len):
    m_len(len)
{
    if (is_ip6)
    {
        std::array<uint8_t, 16> a;
        std::copy(addr, addr+16, std::begin(a));
        boost::asio::ip::address_v6 v6(a);
        m_addr = v6;
    }
    else
    {
        std::array<uint8_t, 4> a;
        std::copy(addr, addr+4, std::begin(a));
        boost::asio::ip::address_v4 v4(a);
        m_addr = v4;
    }
}

void Route::prefix_t::to_vpp(uint8_t *is_ip6,
                             uint8_t *addr,
                             uint8_t *len) const
{
    *is_ip6 = m_addr.is_v6();
    *len = m_len;
    if (m_addr.is_v6())
    {
        *is_ip6 = 1;
        memcpy(addr, m_addr.to_v6().to_bytes().data(), 16);
    }
    else
    {
        *is_ip6 = 0;
        memcpy(addr, m_addr.to_v4().to_bytes().data(), 4);
    }
}

Route::prefix_t Route::prefix_t::from_string(const std::string &str)
{
    std::vector<std::string> strs;

    boost::split(strs, str, boost::is_any_of("/"));

    return (Route::prefix_t(boost::asio::ip::address::from_string(strs[0]),
                            std::stoi(strs[1])));
}
