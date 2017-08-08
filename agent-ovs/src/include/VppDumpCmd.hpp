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

#include <vapi/vapi.hpp>

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
    template <typename MSG>
    class DumpCmd: public Cmd
    {
    public:
        typedef MSG msg_t;
        typedef typename MSG::resp_type record_t;

        typedef typename vapi::Result_set<typename MSG::resp_type>::const_iterator const_iterator;

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

        const_iterator begin()
        {
            return (m_dump->get_result_set().begin());
        }

        const_iterator end()
        {
            return (m_dump->get_result_set().end());
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
         * Call operator called when the dump is complete
         */
        vapi_error_e operator() (MSG &d)
        {
            m_promise.set_value(rc_t::OK);

            return (VAPI_OK);
       }

    protected:
        /**
         * The underlying promise that implements the synchornous nature
         * of the command issue
         */
        std::promise<rc_t> m_promise;

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

        /**
         * The VAPI event registration
         */
        std::unique_ptr<MSG> m_dump;
   };
};


#endif
