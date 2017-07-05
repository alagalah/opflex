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
    template <typename T>
    class EventCmd
    {
    public:
        EventCmd()
        {
        }

        virtual ~EventCmd()
        {
        }

        virtual void notify(T *data) = 0;

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

    protected:

        template <typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     T *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::INFO) << cmd->to_string();
            {
                std::lock_guard<std::mutex> s(cmd->m_mutex);
                cmd->m_events.push(*reply);
            }

            cmd->notify(reply);

            return (VAPI_OK);     
        }

        template <typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     void *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::INFO) << cmd->to_string();
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
