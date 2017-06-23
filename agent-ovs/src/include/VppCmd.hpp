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

extern "C"
{
    #include "vapi.h"
}

namespace VPP
{
    class Connection;

    /**
     * A representation of a method call to VPP
     */
    class Cmd
    {
    public:
        Cmd()
        {
        }
        virtual ~Cmd()
        {
        }

        virtual rc_t issue(Connection &con) = 0;
        virtual void retire();

        virtual std::string to_string() const = 0;

    };

    /**
     * Free ostream function to print a command
     */
    std::ostream & operator<<(std::ostream &os, const Cmd &cmd);
};


#endif
