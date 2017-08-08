/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppAclBinding.hpp"

DEFINE_VAPI_MSG_IDS_ACL_API_JSON;

namespace VPP
{
    namespace ACL
    {
        template<> rc_t L3Binding::BindCmd::issue(Connection &con)
        {
            msg_t req(con.ctx(), std::ref(*this));

            auto &payload = req.get_request().get_payload();
            payload.sw_if_index = m_itf.value();
            payload.is_add = 1;
            payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            payload.acl_index = m_acl.value();

            VAPI_CALL(req.execute());

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template<> rc_t L3Binding::UnbindCmd::issue(Connection &con)
        {
            msg_t req(con.ctx(), std::ref(*this));

            auto &payload = req.get_request().get_payload();
            payload.sw_if_index = m_itf.value();
            payload.is_add = 0;
            payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            payload.acl_index = m_acl.value();

            VAPI_CALL(req.execute());

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template <> rc_t L3Binding::DumpCmd::issue(Connection &con)
        {
            m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

            auto &payload = m_dump->get_request().get_payload();
            payload.sw_if_index = ~0;

            VAPI_CALL(m_dump->execute());

            wait();

            return rc_t::OK;
        }

        template<> rc_t L2Binding::BindCmd::issue(Connection &con)
        {
            msg_t req(con.ctx(), std::ref(*this));

            auto &payload = req.get_request().get_payload();
            payload.sw_if_index = m_itf.value();
            payload.is_add = 1;
            // payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            payload.acl_index = m_acl.value();

            VAPI_CALL(req.execute());

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template<> rc_t L2Binding::UnbindCmd::issue(Connection &con)
        {
            msg_t req(con.ctx(), std::ref(*this));

            auto &payload = req.get_request().get_payload();
            payload.sw_if_index = m_itf.value();
            payload.is_add = 0;
            // payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            payload.acl_index = m_acl.value();

            VAPI_CALL(req.execute());

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template <> rc_t L2Binding::DumpCmd::issue(Connection &con)
        {
            m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

            auto &payload = m_dump->get_request().get_payload();
            payload.sw_if_index = ~0;

            VAPI_CALL(m_dump->execute());

            wait();

            return rc_t::OK;
        }
    }
}
