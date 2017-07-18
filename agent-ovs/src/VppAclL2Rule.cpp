/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>

#include "VppAclL2Rule.hpp"

using namespace VPP::ACL;

L2Rule::L2Rule(uint32_t priority,
               const action_t &action,
               const Route::prefix_t &ip,
               const mac_address_t &mac,
               const mac_address_t &mac_mask):
    m_priority(priority),
    m_action(action),
    m_src_ip(ip),
    m_mac(mac),
    m_mac_mask(mac_mask)
{
}

bool L2Rule::operator<(const L2Rule &other) const
{
    return (m_priority < other.m_priority);
}

void L2Rule::to_vpp(vapi_type_macip_acl_rule &rule) const
{
    rule.is_permit = m_action.value();
    m_src_ip.to_vpp(&rule.is_ipv6, rule.src_ip_addr, &rule.src_ip_prefix_len);
    m_mac.to_bytes(rule.src_mac, 6);
    m_mac_mask.to_bytes(rule.src_mac_mask, 6);
}

bool L2Rule::operator==(const L2Rule &rule) const
{
    return ((m_action == rule.m_action) &&
            (m_src_ip == rule.m_src_ip) &&
            (m_mac == rule.m_mac) &&
            (m_mac_mask == rule.m_mac_mask));
}

std::string L2Rule::to_string() const
{
    std::ostringstream s;

    s << "L2-rule:["
      << "priority:"  << m_priority
      << " action:"   << m_action.to_string()
      << " ip:"       << m_src_ip.to_string()
      << " mac:"      << m_mac
      << " mac-mask:" << m_mac_mask
      << "]";

    return (s.str());  
}
