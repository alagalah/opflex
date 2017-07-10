/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppAclBinding.hpp"

namespace VPP
{
    namespace ACL
    {
        template<> rc_t L3Binding::BindCmd::issue(Connection &con)
        {
            vapi_msg_acl_interface_add_del *req;

            req = vapi_alloc_acl_interface_add_del(con.ctx());
            req->payload.sw_if_index = m_itf.value();
            req->payload.is_add = 1;
            req->payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            req->payload.acl_index = m_acl.value();

            VAPI_CALL(vapi_acl_interface_add_del(
                          con.ctx(), req,
                          RpcCmd::callback<vapi_payload_acl_interface_add_del_reply,
                                           BindCmd>,
                          this));

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template<> rc_t L3Binding::UnbindCmd::issue(Connection &con)
        {
            vapi_msg_acl_interface_add_del *req;

            req = vapi_alloc_acl_interface_add_del(con.ctx());
            req->payload.sw_if_index = m_itf.value();
            req->payload.is_add = 0;
            req->payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            req->payload.acl_index = m_acl.value();

            VAPI_CALL(vapi_acl_interface_add_del(
                          con.ctx(), req,
                          RpcCmd::callback<vapi_payload_acl_interface_add_del_reply,
                                           UnbindCmd>,
                          this));

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template <> rc_t L3Binding::DumpCmd::issue(Connection &con)
        {
            vapi_msg_acl_interface_list_dump *req;

            req = vapi_alloc_acl_interface_list_dump(con.ctx());
            req->payload.sw_if_index = ~0;

            VAPI_CALL(vapi_acl_interface_list_dump(con.ctx(), req,
                                                   DumpCmd::callback<DumpCmd>,
                                                   this));

            wait();

            return rc_t::OK;
        }

        template<> rc_t L2Binding::BindCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_interface_add_del *req;

            req = vapi_alloc_macip_acl_interface_add_del(con.ctx());
            req->payload.sw_if_index = m_itf.value();
            req->payload.is_add = 1;
            // req->payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            req->payload.acl_index = m_acl.value();

            VAPI_CALL(vapi_macip_acl_interface_add_del(
                          con.ctx(), req,
                          RpcCmd::callback<vapi_payload_macip_acl_interface_add_del_reply,
                                           BindCmd>,
                          this));

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        template<> rc_t L2Binding::UnbindCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_interface_add_del *req;

            req = vapi_alloc_macip_acl_interface_add_del(con.ctx());
            req->payload.sw_if_index = m_itf.value();
            req->payload.is_add = 0;
            // req->payload.is_input = (m_direction == direction_t::INPUT ? 1 : 0);
            req->payload.acl_index = m_acl.value();

            VAPI_CALL(vapi_macip_acl_interface_add_del(
                          con.ctx(), req,
                          RpcCmd::callback<vapi_payload_macip_acl_interface_add_del_reply,
                                           UnbindCmd>,
                          this));

            m_hw_item.set(wait());

            return rc_t::OK;
        }

        uword vapi_calc_macip_acl_interface_get_reply_payload_size(vapi_payload_macip_acl_interface_get_reply *payload)
        {
            return sizeof(*payload)+ payload->count * sizeof(payload->acls[0]);
        }

        template <> rc_t L2Binding::DumpCmd::issue(Connection &con)
        {
            vapi_msg_macip_acl_interface_get *req;

            req = vapi_alloc_macip_acl_interface_get(con.ctx());

            VAPI_CALL(vapi_macip_acl_interface_get(
                          con.ctx(), req,
                          DumpCmd::reply_vl<DumpCmd>,
                          mk_cb_ctx(this,
                                    vapi_calc_macip_acl_interface_get_reply_payload_size)));

            wait();

            return rc_t::OK;
        }
    }
}
