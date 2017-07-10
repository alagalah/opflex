/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppAclList.hpp"

namespace VPP
{
    namespace ACL
    {
        template <> void L2List::populate(const KEY &key)
        {
            /*
             * dump VPP Bridge domains
             */
            L2List::DumpCmd::details_type *record;
            std::shared_ptr<L2List::DumpCmd> cmd(new L2List::DumpCmd());

            HW::enqueue(cmd);
            HW::write();

            while (record = cmd->pop())
            {
                const handle_t hdl(record->acl_index);
                L2List acl(hdl, std::string(reinterpret_cast<const char*>(record->tag)));

                for (int ii = 0; ii < record->count; ii++)
                {
                    const Route::prefix_t pfx(record->r[0].is_ipv6,
                                              record->r[0].src_ip_addr,
                                              record->r[0].src_ip_prefix_len);
                    L2Rule rule(ii,
                                action_t::from_int(record->r[0].is_permit),
                                pfx,
                                {record->r[0].src_mac},
                                {record->r[0].src_mac_mask});

                    acl.insert(rule);
                }
                LOG(ovsagent::DEBUG) << "dump: " << acl.to_string();

                /*
                 * Write each of the discovered ACLs into the OM,
                 * but disable the HW Command q whilst we do, so that no
                 * commands are sent to VPP
                 */
                VPP::OM::commit(key, acl);

                free(record);
            }
        }
        template <> void L3List::populate(const KEY &key)
        {
        }
    };
};
