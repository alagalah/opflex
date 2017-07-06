/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppConnection.hpp"

using namespace VPP;

Connection::Connection():
    m_app_name("agent-vpp"),
    m_app_prefix("/vapi-agent-vpp")
{
    vapi_ctx_alloc (&m_ctx);
}

Connection::~Connection()
{
    m_rv = vapi_disconnect(m_ctx);
    vapi_ctx_free(m_ctx);
}

bool Connection::connected() const
{
    return (VAPI_OK == m_rv);
}

void Connection::connect()
{
    do
    {
        m_rv = vapi_connect(m_ctx,
                            m_app_name.c_str(),
                            NULL, //m_api_prefix.c_str(),
                            128,
                            VAPI_MODE_NONBLOCKING);
    } while (VAPI_OK != m_rv);
}

vapi_ctx_t & Connection::ctx()
{
    return (m_ctx);
}
