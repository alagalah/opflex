/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppAclList.hpp"

DEFINE_VAPI_MSG_IDS_ACL_API_JSON;

namespace VPP
{
    namespace ACL
    {
        template<> rc_t L3List::UpdateCmd::issue(Connection &con)
        {
            vapi_msg_acl_add_replace *req;
            uint32_t ii = 0;

            req = vapi_alloc_acl_add_replace(con.ctx(), m_rules.size());
            req->payload.acl_index = m_hw_item.data().value();
            req->payload.count = m_rules.size();
            memset(req->payload.tag, 0, sizeof(req->payload.tag));
            memcpy(req->payload.tag, m_key.c_str(),
                   std::min(m_key.length(),
                            sizeof(req->payload.tag)));

            auto it = m_rules.cbegin();

            while (it != m_rules.cend())
            {
                it->to_vpp(req->payload.r[ii]);
                ++it;
                ++ii;
            }

            VAPI_CALL(vapi_acl_add_replace(
                          con.ctx(), req,
                          L3List::create_callback<vapi_payload_acl_add_replace_reply,
                                                  UpdateCmd>,
                          this));

            m_hw_item = wait();
            complete();

            return rc_t::OK;
        }

        template <> rc_t L3List::DeleteCmd::issue(Connection &con)
        {
            vapi_msg_acl_del *req;

            req = vapi_alloc_acl_del(con.ctx());
            req->payload.acl_index = m_hw_item.data().value();

            VAPI_CALL(vapi_acl_del(con.ctx(),
                                   req,
                                   RpcCmd::callback<vapi_payload_acl_del_reply,
                                   DeleteCmd>,
                                   this));

            wait();
            m_hw_item.set(rc_t::NOOP);

            return rc_t::OK;
        }

        template <> rc_t L3List::DumpCmd::issue(Connection &con)
        {
            vapi_msg_acl_dump *req;

            req = vapi_alloc_acl_dump(con.ctx());
            req->payload.acl_index = ~0;

            VAPI_CALL(vapi_acl_dump(con.ctx(), req,
                                    DumpCmd::callback<DumpCmd>,
                                    this));

            wait();

            return rc_t::OK;
        }

        template<> rc_t L2List::UpdateCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_add *req;
            uint32_t ii = 0;

            req = vapi_alloc_macip_acl_add(con.ctx(), m_rules.size());
            // req->payload.acl_index = m_hw_item.data().value();
            req->payload.count = m_rules.size();
            memset(req->payload.tag, 0, sizeof(req->payload.tag));
            memcpy(req->payload.tag, m_key.c_str(),
                   std::min(m_key.length(),
                            sizeof(req->payload.tag)));

            auto it = m_rules.cbegin();

            while (it != m_rules.cend())
            {
                it->to_vpp(req->payload.r[ii]);
                ++it;
                ++ii;
            }

            VAPI_CALL(vapi_macip_acl_add(
                          con.ctx(), req,
                          L2List::create_callback<vapi_payload_macip_acl_add_reply,
                                                  UpdateCmd>,
                          this));

            m_hw_item = wait();

            return rc_t::OK;
        }

        template <> rc_t L2List::DeleteCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_del *req;

            req = vapi_alloc_macip_acl_del(con.ctx());
            req->payload.acl_index = m_hw_item.data().value();

            VAPI_CALL(vapi_macip_acl_del(con.ctx(),
                                   req,
                                   RpcCmd::callback<vapi_payload_macip_acl_del_reply,
                                                    DeleteCmd>,
                                   this));

            wait();
            m_hw_item.set(rc_t::NOOP);

            return rc_t::OK;
        }

        static uword vapi_calc_macip_acl_details_payload_size(vapi_payload_macip_acl_details *payload)
        {
            return sizeof(*payload) + payload->count * sizeof(payload->r[0]);
        }

        template <> rc_t L2List::DumpCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_dump *req;

            req = vapi_alloc_macip_acl_dump(con.ctx());
            req->payload.acl_index = ~0;

            VAPI_CALL(vapi_macip_acl_dump(con.ctx(), req,
                                          DumpCmd::callback_vl<DumpCmd>,
                                          mk_cb_ctx(this,
                                                    vapi_calc_macip_acl_details_payload_size)));

            wait();

            return rc_t::OK;
        }
    }
}
