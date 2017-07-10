/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_L3_ACL_RULE_H__
#define __VPP_L3_ACL_RULE_H__

#include <stdint.h>

#include "VppAclTypes.hpp"
#include "VppRoute.hpp"

extern "C"
{
    #include "acl.api.vapi.h"
}

namespace VPP
{
    namespace ACL
    {
        /**
         * An ACL rule is the building block of an ACL. An ACL, which is
         * the object applied to an interface, is comprised of an ordersed
         * sequence of ACL rules.
         * This class is a wrapper around the VAPI generated struct and exports
         * an API with better types.
         */
        class L3Rule
        {
        public:
            /**
             * Construct a new object matching the desried state
             */
            L3Rule(uint32_t priority,
                   const action_t &action,
                   const Route::prefix_t &src,
                   const Route::prefix_t &dst);
        
            /**
             * Copy Constructor
             */
            L3Rule(const L3Rule& o) = default;

            /**
             * Destructor
             */
            ~L3Rule() = default;

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * less-than operator
             */
            bool operator<(const L3Rule &rule) const;

            /**
             * comparison operator (for testing)
             */
            bool operator==(const L3Rule &rule) const;

            /**
             * Convert to VPP API fromat
             */
            void to_vpp(vapi_type_acl_rule &rule) const;

        private:
            /**
             * Priority. Used to sort the rules in a list in the order
             * in which they are applied
             */
            uint32_t m_priority;

            /**
             * Action on match
             */
            action_t m_action;

            /**
             * Source Prefix
             */
            Route::prefix_t m_src;

            /**
             * Destination Prefix
             */
            Route::prefix_t m_dst;

            /**
             * L4 protocol. IANA number. 1 = ICMP, 58 = ICMPv6, 6 = TCP, 17 = UDP.
             * 0 => ignore L4 and ignore the ports/tcpflags when matching.
             */
            uint8_t proto;

            /**
             * If the L4 protocol is TCP or UDP, the below
             * hold ranges of ports, else if the L4 is ICMP/ICMPv6
             * they hold ranges of ICMP(v6) types/codes.
             *
             * Ranges are inclusive, i.e. to match "any" TCP/UDP port,
             * use first=0,last=65535. For ICMP(v6),
             * use first=0,last=255.
             */
            uint16_t srcport_or_icmptype_first;
            uint16_t srcport_or_icmptype_last;
            uint16_t dstport_or_icmpcode_first;
            uint16_t dstport_or_icmpcode_last;

            /*
             * for proto = 6, this matches if the
             * TCP flags in the packet, ANDed with tcp_flags_mask,
             * is equal to tcp_flags_value.
             */
            uint8_t tcp_flags_mask;
            uint8_t tcp_flags_value;
        };
    };
};

#endif
