/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_EVENT_CMD_H__
#define __VPP_EVENT_CMD_H__

#include <queue>
#include <mutex>

#include "logging.h"

#include "VppCmd.hpp"

namespace VPP
{
    /**
     * An Event command base class.
     * Events are one of the sub-set of command type to VPP.
     * A client performs a one time 'registration/subsription' to VPP for the
     * event in question and then is notified asynchronously when those events
     * occur. 
     * The model here then is that the lifetime of the event command represensts
     * the during of the clients subscription. When the command is 'issued' the
     * subscription begins, when it is 'retired' the subscription ends. For the
     * subscription duration the client will be notified as events are recieved.
     * The client can then 'pop' these events from this command object.
     */
    template <typename T>
    class EventCmd
    {
    public:
        /**
         * Default constructor
         */
        EventCmd()
        {
        }

        /**
         * Default destructor
         */
        virtual ~EventCmd()
        {
        }

        /**
         * pop (consume) an event from VPP that was a result of this commands
         * subscription
         */
        bool pop(T &data)
        {
            std::lock_guard<std::mutex> lg(m_mutex);

            if (!m_events.size())
            {
                return false;
            }

            data = m_events.front();
            m_events.pop();

            return true;
        }

        /**
         * Retire the command. This is only appropriate for Event Commands
         * As they persist until retired.
         */
        virtual void retire()
        {
        }

    protected:
        /**
         * Notify the command that data from VPP has arrived and been stored.
         * The command should now inform its clients/listeners.
         */
        virtual void notify(T *data) = 0;

        /**
         * A Callback registered with VPP API that is invoked when an event
         * arrives. This is the type-specific version that can be used with VAPI
         * when the type can be specified.
         */
        template <typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     T *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::DEBUG) << cmd->to_string();

            {
                std::lock_guard<std::mutex> s(cmd->m_mutex);
                cmd->m_events.push(*reply);
            }

            cmd->notify(reply);

            return (VAPI_OK);     
        }

        /**
         * A Callback registered with VPP API that is invoked when an event
         * arrives. This is the type-specific version that can be used with VAPI
         * when the type cannot be specified.
         */
        template <typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     void *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::DEBUG) << cmd->to_string();

            {
                std::lock_guard<std::mutex> s(cmd->m_mutex);
                T *data = static_cast<T*>(reply);

                cmd->m_events.push(*data);
            }
            cmd->notify(static_cast<T*>(reply));

            return (VAPI_OK);     
        }

        /**
         * The event command can be called many times before
         * the listener gets a chance to read. so we need to
         * queue up the data recevied.
         */
        std::queue<T> m_events;

        /**
         * Mutex protection for the events
         */
        std::mutex m_mutex;
    };
};


#endif
