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
    template <typename HWITEM, typename DATA>
    class RpcCmd: public Cmd
    {
    public:
        RpcCmd(HWITEM &item):
            Cmd(),
            m_hw_item(item)
        {
        }

        virtual ~RpcCmd()
        {
        }

        HWITEM & item()
        {
            return m_hw_item;
        }
        const HWITEM & item() const
        {
            return m_hw_item;
        }

        void fulfill(const DATA &d)
        {
            m_promise.set_value(d);
        }

        DATA wait()
        {
            return (m_promise.get_future().get());
        }

    protected:
        HWITEM &m_hw_item;

        std::promise<DATA> m_promise;

        template <typename REPLY, typename CMD_TYPE>
        static vapi_error_e callback(vapi_ctx_t ctx,
                                     void *callback_ctx,
                                     vapi_error_e rv,
                                     bool is_last,
                                     REPLY *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::INFO) << cmd->to_string() << " " << reply->retval;
            cmd->fulfill(rc_t::from_vpp_retval(reply->retval));

            return (VAPI_OK);     
        }
    };
};


#endif
