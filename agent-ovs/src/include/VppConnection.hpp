/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Include file for VppConnection
 *
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_CONNECTION_H__
#define __VPP_CONNECTION_H__

#include <string>
#include <mutex>

extern "C"
{
#include "vapi.h"
}

namespace VPP {
    /**
     * A representation of the connection to VPP
     */
    class Connection
    {
    public:
        /**
         * Constructor
         */
        Connection();
        /**
         * Destructor
         */
        ~Connection();

        /**
         * Blocking connect call - always eventually succeeds, or the
         * universe expires. Not much this system can do without one.
         */
        void connect();

        /**
         * Retrun the VAPI context the commands will use
         */
        vapi_ctx_t & ctx();
    private:
        /**
         * The VAPI connection context
         */
        vapi_ctx_t m_ctx;

        /**
         * The name of this application
         */
        const std::string m_app_name;
    };
};

#endif
