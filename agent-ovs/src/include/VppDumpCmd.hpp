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

namespace VPP
{
    template <typename T>
    class DumpCmd: public Cmd
    {
    public:
        DumpCmd():
            Cmd()
        {
        }

        virtual ~DumpCmd()
        {
        }

        bool pop(T &data)
        {
            if (!m_events.size())
            {
                return false;
            }

            data = m_events.front();
            m_events.pop();

            return true;
        }

        rc_t wait()
        {
            return (m_promise.get_future().get());
        }

        typedef T details_type;

    protected:
        std::promise<rc_t> m_promise;

        template <typename DERIVED>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     vapi_error_e rv,
                                     bool is_last,
                                     T *reply)
        {
            //Cmd *c = static_cast<Cmd*>(callback_ctx);
            DERIVED *cmd = static_cast<DERIVED*>(callback_ctx);

            LOG(ovsagent::INFO) << "last:" << is_last << " " << cmd->to_string();
            if (is_last)
            {
                cmd->m_promise.set_value(rc_t::OK);
            }
            else
            {
                cmd->m_events.push(*reply);
            } 

            return (VAPI_OK);     
        }

        /**
         * The dump command receives many 'details' before the command
         * completes. save them in the order they arrived.
         */
        std::queue<T> m_events;
   };
};


#endif
