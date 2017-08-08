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
    m_app_name("vpp-OM")
{
}

Connection::~Connection()
{
    disconnect();
}

void Connection::disconnect()
{
    m_vapi_conn.disconnect();
}

void Connection::connect()
{
    vapi_error_e rv;

    do
    {
        rv = m_vapi_conn.connect(m_app_name.c_str(),
                                 NULL, //m_api_prefix.c_str(),
                                 128,
                                 128);
    } while (VAPI_OK != rv);
}

vapi::Connection & Connection::ctx()
{
    return (m_vapi_conn);
}
