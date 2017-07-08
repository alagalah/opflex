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
    /**
     * A means to inspect the state VPP has built, in total, and per-client
     * To use do:
     *   socat - UNIX-CONNECT:/path/to/sock/in/opflex.conf
     * and follow the instructions
     */
    class Inspect
    {
    public:
        /**
         * Constructor
         */
        Inspect(const std::string &sockname);

        /**
         * Destructor to tidyup socket resources
         */
        ~Inspect();

    private:
        /**
         * Call operator for running in the thread
         */
        static void run(void* ctx);

        /**
         * Base class for all the command objects
         */
        class Command
        {
        public:
            /**
             * Virtual Destructor
             */
            virtual ~Command() = default;

            /**
             * Execute the command - capture output in ostream
             */
            virtual void exec(std::ostream &os) = 0;
        protected:
            Command() = default;
        };

        /**
         * Show All object types
         */
        class ShowAll: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowAll() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All Interfaces
         */
        class ShowInterface: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowInterface() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All Bridge-Domains
         */
        class ShowBridgeDomain: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowBridgeDomain() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All Route-Domains
         */
        class ShowRouteDomain: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowRouteDomain() = default;
            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All L3 Configs
         */
        class ShowL3Config: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowL3Config() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All L2 Configs
         */
        class ShowL2Config: public Command
        {
        public:
            /**
             * Default Constructor
             */
            ShowL2Config() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Show All Objects referenced by a given KEY
         */
        class ShowKey: public Command
        {
        public:
            /**
             * Constructor
             */
            ShowKey(const std::string &key);

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        private:
            /**
             * The KEY owning the objects to display
             */
            std::string m_key;
        };

        /**
         * Show Help
         */
        class ShowHelp: public Command
        {
        public:
            /**
             * Constructor
             */
            ShowHelp() = default;

            /**
             * Execute the command - capture output in ostream
             */
            void exec(std::ostream &os);
        };

        /**
         * Construct a command from the message on the socket
         */
        static Command* new_command(const std::string &command);

        /**
         * A write request
         */
        struct write_req_t
        {
            write_req_t(std::ostringstream &output);
            ~write_req_t();

            uv_write_t req;
            uv_buf_t buf;
        };

        /**
         * Write a ostream to the client
         */
        static void do_write(uv_stream_t *client, std::ostringstream &output);

        /**
         * Called on creation of a new connection
         */
        static void on_connection(uv_stream_t* server,
                                  int status);

        /**
         * Call when data is written
         */
        static void on_write(uv_write_t *req, int status);

        /**
         * Called when data is read
         */
        static void on_read(uv_stream_t *client,
                            ssize_t nread,
                            const uv_buf_t *buf);

        /**
         * Called to allocate buffer space for data to be read
         */
        static void on_alloc_buffer(uv_handle_t *handle,
                                    size_t size,
                                    uv_buf_t *buf);

        /**
         * Called to cleanup the thread and socket during destruction
         */
        static void on_cleanup(uv_async_t* handle);

        /**
         * Async handle so we can wakeup the loop
         */
        uv_async_t m_async;

        /**
         * The libuv loop
         */
        uv_loop_t m_server_loop;

        /**
         * The libuv thread context in which we run the loop
         */
        uv_thread_t m_server_thread;

        /**
         * The inspect unix domain socket name, from the config file
         */
        std::string m_sock_name;
    };
};

#endif
