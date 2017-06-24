/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppInspect.hpp"

using namespace VPP;

Inspect::Inspect()
{
    int rc;
    uv_loop_init(&server_loop);
    cleanup_async.data = this;
    writeq_async.data = this;
    uv_async_init(&server_loop, &cleanup_async, on_cleanup_async);
    uv_async_init(&server_loop, &writeq_async, on_writeq_async);

    //yajr::initLoop(&server_loop);

    /* if (port < 0) { */
    /*     listener = */
    /*         yajr::Listener::create(socketName, */
    /*                                OpflexServerConnection::on_state_change, */
    /*                                on_new_connection, */
    /*                                this, */
    /*                                &server_loop, */
    /*                                OpflexServerConnection::loop_selector); */
    /* } else { */
    /*     listener = */
    /*         yajr::Listener::create("0.0.0.0", port, */
    /*                                OpflexServerConnection::on_state_change, */
    /*                                on_new_connection, */
    /*                                this, */
    /*                                &server_loop, */
    /*                                OpflexServerConnection::loop_selector); */
    /* } */

    /* rc = uv_thread_create(&server_thread, run, this); */
    /* if (rc < 0) { */
    /*     throw std::runtime_error(string("Could not create server thread: ") + */
    /*                              uv_strerror(rc)); */
    /* } */
}
