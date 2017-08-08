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

uint8_t Route::prefix_t::mask_width() const
{
    return (m_len);
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

boost::asio::ip::address VPP::from_bytes(uint8_t is_ip6, uint8_t *bytes)
{
    boost::asio::ip::address addr;

    if (is_ip6)
    {
        std::array<uint8_t, 16> a;
        std::copy(bytes, bytes+16, std::begin(a));
        boost::asio::ip::address_v6 v6(a);
        addr = v6;
    }
    else
    {
        std::array<uint8_t, 4> a;
        std::copy(bytes, bytes+4, std::begin(a));
        boost::asio::ip::address_v4 v4(a);
        addr = v4;
    }

    return (addr);
}

Route::prefix_t::prefix_t(uint8_t is_ip6,
                          uint8_t *addr,
                          uint8_t len):
    m_len(len),
    m_addr(from_bytes(is_ip6, addr))
{
}

const boost::asio::ip::address_v4  VPP::operator|(
                        const boost::asio::ip::address_v4 &addr1,
                        const boost::asio::ip::address_v4 &addr2)
{
    uint32_t a;
    a = addr1.to_ulong() | addr2.to_ulong();
    boost::asio::ip::address_v4 addr(a);
    return (addr);
}

const boost::asio::ip::address_v4 VPP::operator&(
                        const boost::asio::ip::address_v4 &addr1,
                        const boost::asio::ip::address_v4 &addr2)
{
    uint32_t a;
    a = addr1.to_ulong() & addr2.to_ulong();
    boost::asio::ip::address_v4 addr(a);
    return (addr);
}

const boost::asio::ip::address_v4 VPP::operator~(
                        const boost::asio::ip::address_v4 &addr1)
{
    uint32_t a;
    a = ~addr1.to_ulong();
    boost::asio::ip::address_v4 addr(a);
    return (addr);
}

const boost::asio::ip::address_v4 VPP::mask(const Route::prefix_t &pfx)
{
    uint32_t a;

    a = ~ ((1 << pfx.mask_width()) - 1);
    boost::asio::ip::address_v4 addr(a);
    return (addr);
}

const boost::asio::ip::address_v4 VPP::low(const Route::prefix_t &pfx)
{
    boost::asio::ip::address_v4 low;
    low = pfx.address().to_v4() & mask(pfx);
    return (low);
}

const boost::asio::ip::address_v4 VPP::high(const Route::prefix_t &pfx)
{
    boost::asio::ip::address_v4 high;
    high = pfx.address().to_v4() | ~mask(pfx);
    return (high);
}

void VPP::to_bytes(const boost::asio::ip::address &addr,
                   uint8_t *is_ip6,
                   uint8_t *array)
{
    *is_ip6 = addr.is_v6();
    if (addr.is_v6())
    {
        *is_ip6 = 1;
        memcpy(array, addr.to_v6().to_bytes().data(), 16);
    }
    else
    {
        *is_ip6 = 0;
        memcpy(array, addr.to_v4().to_bytes().data(), 4);
    }
}

void Route::prefix_t::to_vpp(uint8_t *is_ip6,
                             uint8_t *addr,
                             uint8_t *len) const
{
    *len = m_len;
    to_bytes(m_addr, is_ip6, addr);
}

std::ostream & VPP::operator<<(std::ostream &os,
                               const Route::prefix_t &pfx)
{
    os << pfx.to_string();

    return (os);
}
