/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_SUB_INTERFACE_H__
#define __VPP_SUB_INTERFACE_H__

#include "VppInterface.hpp"

namespace VPP
{
    /**
     * A Sub-interface. e.g. a VLAN sub-interface on an Ethernet interface
     */
    class SubInterface: public Interface
    {
        /*
         * Typedef for VLAN ID
         */
        typedef u32 vlan_id_t;

    public:
        /**
         * Construct a new object matching the desried state
         */
        SubInterface(const std::string &name,
                     const std::shared_ptr<Interface> parent,
                     admin_state_t state,
                     vlan_id_t vlan);
        ~SubInterface();
        SubInterface(const SubInterface& o);

        /**
         * Debug rpint function
         */
        std::string to_string() const;

    private:
        /**
         * VLAN ID
         */
        vlan_id_t m_vlan;

        /**
         * Refernece conter lock on the parent
         */
        const std::shared_ptr<Interface> m_parent;
   };
};
#endif
