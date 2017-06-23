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
        Connection();
        ~Connection();

        bool connected() const;

        void connect();

        vapi_ctx_t & ctx();
    private:
        vapi_ctx_t m_ctx;
        vapi_error_e m_rv;
        const std::string m_app_name;
        const std::string m_app_prefix;
    };
};

#endif
