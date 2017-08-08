/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_CMD_H__
#define __VPP_CMD_H__

#include <stdint.h>
#include <string>
#include <future>
#include <queue>
#include <mutex>

#include "Vpp.hpp"

#include <vapi/vapi.hpp>

namespace VPP
{
    /**
     * Forward declaration of the VPP::Connection class
     */
    class Connection;

    /**
     * A representation of a method call to VPP
     */
    class Cmd
    {
    public:
        /**
         * Default constructor
         */
        Cmd()
        {
        }
        /**
         * Virtual destructor
         */
        virtual ~Cmd()
        {
        }

        /**
         * Issue the command to VPP/HW
         */
        virtual rc_t issue(Connection &con) = 0;

        /**
         * Invoked on a Command when the HW queue is disabled to indicate
         * that the commnad can be considered successful
         */
        virtual void succeeded() = 0;

        /**
         * convert to string format for debug purposes
         */
        virtual std::string to_string() const = 0;
    };

    /**
     * Free ostream function to print a command
     */
    std::ostream & operator<<(std::ostream &os, const Cmd &cmd);
};


#endif
