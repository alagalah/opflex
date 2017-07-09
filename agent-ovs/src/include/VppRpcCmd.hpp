/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_RPC_CMD_H__
#define __VPP_RPC_CMD_H__

#include <future>

#include "VppCmd.hpp"
#include "logging.h"

namespace VPP
{
    /**
     * A base class for all RPC commands to VPP.
     *  RPC commands are one of the sub-set of command types to VPP
     * that modify/create state in VPP and thus return an error code.
     * Commands are issued in one thread context, but read in another. The
     * command has an associated std::promise that is met by the RX thread.
     * this allows the sender, which waits on the promise's future, to
     * experience a synchronous command.
     *
     * The command is templatised on the type of the HW::Item to be set by
     * the command, and the data returned in the promise,
     */
    template <typename HWITEM, typename DATA>
    class RpcCmd: public Cmd
    {
    public:
        /**
         * Constructor taking the HW item that will be updated by the command
         */
        RpcCmd(HWITEM &item):
            Cmd(),
            m_hw_item(item)
        {
        }

        /**
         * Desructor
         */
        virtual ~RpcCmd()
        {
        }

        /**
         * return the HW item the command updates
         */
        HWITEM & item()
        {
            return m_hw_item;
        }

        /**
         * return the const HW item the command updates
         */        
        const HWITEM & item() const
        {
            return m_hw_item;
        }

        /**
         * Fulfill the commands promise. Called from the RX thread
         */
        void fulfill(const DATA &d)
        {
            m_promise.set_value(d);
        }

        /**
         * Wait on the commands promise. i.e. block on the completion
         * of the command.
         */
        DATA wait()
        {
            std::future_status status;
            std::future<DATA> result;

            result = m_promise.get_future();
            status = result.wait_for(std::chrono::seconds(5));

            if (status != std::future_status::ready)
            {
                return (DATA(rc_t::TIMEOUT));
            }

            return (result.get());
        }

        /**
         * Called by the HW Command Q when it is disabled to indicate the
         * command can be considered successful without issuing it to HW
         */
        virtual void succeeded()
        {
            m_hw_item.set(rc_t::OK);
        }

    protected:
        /**
         * A reference to an object's HW::item that the command will update
         */
        HWITEM &m_hw_item;

        /**
         * The promise that implements the synchronous issue
         */
        std::promise<DATA> m_promise;

        /**
         * A templatised call back function that is register with the VPP API
         * and invoked when the commands reply arrives, in the RX thread.
         * The call back context is always the command object that issues the
         * request.
         */
        template <typename REPLY, typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     vapi_error_e rv,
                                     bool is_last,
                                     REPLY *reply)
        {
            /*
             * fulfill the promise to unblock the sender
             */
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::DEBUG) << cmd->to_string() << " " << reply->retval;
            cmd->fulfill(rc_t::from_vpp_retval(reply->retval));

            return (VAPI_OK);     
        }
    };
};


#endif
