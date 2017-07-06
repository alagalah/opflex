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

        /**
         * Wait for the issue of the command to complete
         */
        rc_t wait()
        {
            return (m_promise.get_future().get());
        }

        /**
         * convenient typedef of the reacord type
         */
        typedef T details_type;

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
