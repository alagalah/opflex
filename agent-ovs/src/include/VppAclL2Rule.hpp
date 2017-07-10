/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_L2_ACL_RULE_H__
#define __VPP_L2_ACL_RULE_H__

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
        class L2Rule
        {
        public:
            /**
             * Construct a new object matching the desried state
             */
            L2Rule(uint32_t priority,
                   const action_t &action,
                   const Route::prefix_t &ip,
                   const mac_address_t &mac,
                   const mac_address_t &mac_mask);
        
            /**
             * Copy Constructor
             */
            L2Rule(const L2Rule& o) = default;

            /**
             * Destructor
             */
            ~L2Rule() = default;

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * less-than operator
             */
            bool operator<(const L2Rule &rule) const;

            /**
             * comparison operator (for testing)
             */
            bool operator==(const L2Rule &rule) const;

            /**
             * Convert to VPP API fromat
             */
            void to_vpp(vapi_type_macip_acl_rule &rule) const;

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
            Route::prefix_t m_src_ip;

            /**
             * Source Mac
             */
            mac_address_t m_mac;

            /**
             * Source MAC mask
             */
            mac_address_t m_mac_mask;
        };
    };
};

#endif
