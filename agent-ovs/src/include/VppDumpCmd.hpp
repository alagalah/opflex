/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_DUMP_CMD_H__
#define __VPP_DUMP_CMD_H__

#include <future>

#include "logging.h"

#include "VppCmd.hpp"
#include "VppHW.hpp"

namespace VPP
{
    /**
     * A function type def for calculating a message's size
     */
    typedef unsigned int (*get_msg_size_t)(void*);

    /**
     * A base class for VPP dump commands.
     * Dump commands are one of the sub-set of command types to VPP. Here the client
     * makes a read request on the resource and VPP responds with all the records.
     * This command is executed synchronously. Once complete the client can 'pop'
     * the records from the command object
     */
    template <typename T>
    class DumpCmd: public Cmd
    {
    public:
        /**
         * Default Constructor
         */
        DumpCmd():
            Cmd()
        {
        }

        /**
         * Destructor
         */
        virtual ~DumpCmd()
        {
        }

        /**
         * Pop one of the recieved records
         */
        T* pop()
        {
            T *data;

            if (!m_events.size())
            {
                return (nullptr);
            }

            data = m_events.front();
            m_events.pop();

            return (data);
        }

        /**
         * Wait for the issue of the command to complete
         */
        rc_t wait()
        {
            std::future_status status;
            std::future<rc_t> result;

            result = m_promise.get_future();
            status = result.wait_for(std::chrono::seconds(5));

            if (status != std::future_status::ready)
            {
                return (rc_t::TIMEOUT);
            }

            return (result.get());
        }

        /**
         * convenient typedef of the reacord type
         */
        typedef T details_type;

        /**
         * A call back context required for variable length records
         */
        struct cb_ctx_t
        {
            /**
             * Constructor
             */
            cb_ctx_t(void *obj,
                     get_msg_size_t gsm):
                obj(obj),
                msg_size(gsm)
            {
            }

            /**
             * The command object
             */
            void *obj;

            /**
             * The callback function to get the record message size
             */
            get_msg_size_t msg_size;
        };

        /**
         * Make a call back context
         */
        template <typename U>
        static void *mk_cb_ctx(void *obj, U gsm)
        {
            return (new cb_ctx_t(obj, reinterpret_cast<get_msg_size_t>(gsm)));
        }

    protected:
        /**
         * The underlying promise that implements the synchornous nature
         * of the command issue
         */
        std::promise<rc_t> m_promise;

        /**
         * Callback function registered with the VPP API that will be invoked for
         * each record recieved.
         * the callback context passed is always the command object that issued
         * the command.
         */
        template <typename DERIVED>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     vapi_error_e rv,
                                     bool is_last,
                                     T *reply)
        {
            DERIVED *cmd = static_cast<DERIVED*>(callback_ctx);

            LOG(ovsagent::DEBUG) << "last:" << is_last << " " << cmd->to_string();

            if (is_last)
            {
                cmd->m_promise.set_value(rc_t::OK);
            }
            else
            {
                T *t = (T*) malloc(sizeof(T));

                memcpy(t, reply, sizeof(T));
                cmd->m_events.push(t);
            }

            return (VAPI_OK);
        }

        /**
         * Callback function registered with the VPP API that will be invoked for
         * each record recieved.
         * the callback context passed is a cb_ctx_t used to handle variable length
         * records
         */
        template <typename DERIVED>
        static vapi_error_e callback_vl(vapi_ctx_t ctx,
                                        void *callback_ctx,
                                        vapi_error_e rv,
                                        bool is_last,
                                        T *reply)
        {
            cb_ctx_t *cb_ctx = static_cast<cb_ctx_t*>(callback_ctx);
            DERIVED *cmd = static_cast<DERIVED*>(cb_ctx->obj);

            LOG(ovsagent::DEBUG) << "last:" << is_last << " " << cmd->to_string();

            if (is_last)
            {
                cmd->m_promise.set_value(rc_t::OK);
                delete cb_ctx;
            }
            else
            {
                size_t s = cb_ctx->msg_size(reply);
                T * t = (T*) malloc(s);

                memcpy(t, reply, s);

                cmd->m_events.push(t);
            }

            return (VAPI_OK);
        }

        /**
         * Callback function registered with the VPP API that will be invoked for
         * each record recieved.
         * the callback context passed is always the command object that issued
         * the command.
         * In this variant it is the reply message itself that has the record embedded
         * and there are no individual 'records'/details sent.
         */
        template <typename DERIVED>
        static vapi_error_e reply(vapi_ctx_t ctx,
                                  void *callback_ctx,
                                  vapi_error_e rv,
                                  bool is_last,
                                  T *reply)
        {
            DERIVED *cmd = static_cast<DERIVED*>(callback_ctx);

            LOG(ovsagent::DEBUG) << "last:" << is_last << " " << cmd->to_string();

            T *t = (T*) malloc(sizeof(T));

            memcpy(t, reply, sizeof(T));
            cmd->m_events.push(t);
            cmd->m_promise.set_value(rc_t::OK);

            return (VAPI_OK);
        }

        /**
         * Callback function registered with the VPP API that will be invoked for
         * each record recieved.
         * the callback context passed is the obj and message size calculator
         * In this variant it is the reply message itself that has the record embedded
         * and there are no individual 'records'/details sent.
         */
        template <typename DERIVED>
        static vapi_error_e reply_vl(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     vapi_error_e rv,
                                     bool is_last,
                                     T *reply)
        {
            cb_ctx_t *cb_ctx = static_cast<cb_ctx_t*>(callback_ctx);
            DERIVED *cmd = static_cast<DERIVED*>(cb_ctx->obj);

            LOG(ovsagent::DEBUG) << "last:" << is_last << " " << cmd->to_string();

            size_t s = cb_ctx->msg_size(reply);
            T * t = (T*) malloc(s);

            memcpy(t, reply, s);

            cmd->m_events.push(t);
            cmd->m_promise.set_value(rc_t::OK);
            delete cb_ctx;

            return (VAPI_OK);
        }

        /**
         * The dump command receives many 'details' before the command
         * completes. save them in the order they arrived.
         */
        std::queue<T*> m_events;

        /**
         * Dump commands should not be issued whilst the HW is disabled
         */
        void succeeded()
        {
        }

        /**
         * The HW::CmdQ is a friend so it can call suceedded.
         */
        friend class HW::CmdQ;
   };
};


#endif
