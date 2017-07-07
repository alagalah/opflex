/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_INSPECT_H__
#define __VPP_INSPECT_H__

#include <string>
#include <sstream>
#include <uv.h>

namespace VPP
{
    /*
     * A means to inspect the state VPP has built, in total, and per-client
     */
    class Inspect
    {
    public:
        Inspect(const std::string &sockname);

        /**
         * Call operator for running in the thread
         */
        static void run(void* ctx);

    
    private:
        class Command
        {
            virtual void exec(std::ostringstream &os) = 0;
        };
        class ShowAll: Command
        {
            void exec(std::ostringstream &os);
        };

        class Factory
        {
            static Command* new_command(const std::string &command);
        };

        struct write_req_t
        {
            uv_write_t req;
            uv_buf_t buf;
        };

        static void on_connection(uv_stream_t* server,
                                  int status);
        static void write(uv_write_t *req, int status);
        static void read(uv_stream_t *client,
                         ssize_t nread,
                         const uv_buf_t *buf);
        static void alloc_buffer(uv_handle_t *handle,
                                 size_t suggested_size,
                                 uv_buf_t *buf);

        uv_loop_t m_server_loop;
        uv_thread_t m_server_thread;

        std::string m_sock_name;
    };
};

#endif
