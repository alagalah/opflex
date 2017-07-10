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
        template <> void L2Binding::populate(const KEY &key)
        {
            /*
             * dump VPP Bridge domains
             */
            L2Binding::DumpCmd::details_type *record;
            std::shared_ptr<L2Binding::DumpCmd> cmd(new L2Binding::DumpCmd());

            HW::enqueue(cmd);
            HW::write();

            while (record = cmd->pop())
            {
                for (int ii = 0; ii < record->count; ii++)
                {
                    if (~0 == record->acls[ii]) continue;

                    std::shared_ptr<Interface> itf = Interface::find(ii);
                    std::shared_ptr<L2List> acl = L2List::find(record->acls[ii]);

                    L2Binding binding(direction_t::INPUT, *itf, *acl);

                    OM::commit(key, binding);
                }

                free(record);
            }
        }

        template <> void L3Binding::populate(const KEY &key)
        {
        }
    };

    std::ostream &operator<<(std::ostream &os,
                             const std::pair<ACL::direction_t, Interface::key_type> &key)
    {
        os << "["
           << key.first.to_string()
           << " "
           << key.second
           << "]";

        return (os);
    }
};
