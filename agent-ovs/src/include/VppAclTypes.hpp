/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Main implementation for OVS agent
 *
 * Copyright (c) 2014 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ACL_TYPES_H__
#define __VPP_ACL_TYPES_H__

#include "Vpp.hpp"

namespace VPP
{
    namespace ACL
    {
        /**
         * ACL Actions
         */
        struct action_t: public Enum<action_t>
        {
            /**
             * Constructor
             */
            action_t(int v, const std::string s);

            /**
             * Destructor
             */
            ~action_t() = default;

            /**
             * Permit Action
             */
            const static action_t PERMIT;

            /**
             * Deny Action
             */
            const static action_t DENY;

            /**
             * Get the enum type from a VPP integer value
             */
            static const action_t &from_int(uint8_t i);
        };

        /**
         * ACL Directions
         */
        struct direction_t: public Enum<direction_t>
        {
            /**
             * Constructor
             */
            direction_t(int v, const std::string s);

            /**
             * Destructor
             */
            ~direction_t() = default;

            /**
             * Permit Direction
             */
            const static direction_t INPUT;

            /**
             * Deny Direction
             */
            const static direction_t OUTPUT;
        };
   };
};

#endif
