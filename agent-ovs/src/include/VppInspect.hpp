/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_INSPECT_H__
#define __VPP_INSPECT_H__

#include <thread>
#include <uv.h>

namespace VPP
{
    /*
     * A means to inspect the state VPP has built, in total, and per-client
     */
    class Inspect
    {
    public:
        Inspect();

        /**
         * Call operator for running in the thread
         */
        static void run();

    private:
        uv_loop_t server_loop;
        uv_thread_t server_thread;
        uv_async_t cleanup_async;
        uv_async_t writeq_async;

        static void on_cleanup_async(uv_async_t *handle);
        static void on_writeq_async(uv_async_t *handle);
    };
};

#endif
