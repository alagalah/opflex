/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <sstream>

#include "VppAclL3Rule.hpp"

using namespace VPP::ACL;

L3Rule::L3Rule(uint32_t priority,
               const action_t &action,
               const Route::prefix_t &src,
               const Route::prefix_t &dst):
    m_priority(priority),
    m_action(action),
    m_src(src),
    m_dst(dst)
{
}

bool L3Rule::operator<(const L3Rule &other) const
{
    return (m_priority < other.m_priority);
}

void L3Rule::to_vpp(vapi_type_acl_rule &rule) const
{
    rule.is_permit = m_action.value();
    m_src.to_vpp(&rule.is_ipv6, rule.src_ip_addr, &rule.src_ip_prefix_len);
    m_dst.to_vpp(&rule.is_ipv6, rule.dst_ip_addr, &rule.dst_ip_prefix_len);

    rule.proto = proto;
    rule.srcport_or_icmptype_first = srcport_or_icmptype_first;
    rule.srcport_or_icmptype_last  = srcport_or_icmptype_last;
    rule.dstport_or_icmpcode_first = dstport_or_icmpcode_first;
    rule.dstport_or_icmpcode_last  = dstport_or_icmpcode_last;

    rule.tcp_flags_mask  = tcp_flags_mask;
    rule.tcp_flags_value = tcp_flags_value;;
}

bool L3Rule::operator==(const L3Rule &rule) const
{
    return ((m_priority == rule.m_priority) &&
            (m_action == rule.m_action) &&
            (m_src == rule.m_src) &&
            (m_dst == rule.m_dst));
}

std::string L3Rule::to_string() const
{
    std::ostringstream s;

    s << "L3-rule:["
      << "priority:" << m_priority
      << " action:"  << m_action.to_string()
      << " src:"     << m_src.to_string()
      << " dst:"     << m_dst.to_string()
      << "]";

    return (s.str());  
}