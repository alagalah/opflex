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
        SubInterface(const Interface &parent,
                     admin_state_t state,
                     vlan_id_t vlan);
        ~SubInterface();
        SubInterface(const SubInterface& o);

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public CmdT<HW::Item<handle_t> >
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      handle_t parent,
                      uint16_t vlan);

            rc_t exec();
            std::string to_string() const;

            bool operator==(const CreateCmd&i) const;
        private:
            handle_t m_parent;
            uint16_t m_vlan;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public CmdT<HW::Item<handle_t> >
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item);

            rc_t exec();
            std::string to_string() const;

            bool operator==(const DeleteCmd&i) const;
        };

    private:
        /**
         * Virtual functions to construct an interface create/delete commands.
         */
        virtual Cmd* mk_create_cmd();
        virtual Cmd* mk_delete_cmd();

        static std::shared_ptr<SubInterface> find_or_add(const SubInterface &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
         * From the name of the parent and the vlan,
         * construct the sub-interface's name
         */
        static std::string mk_name(const Interface &parent,
                                   vlan_id_t vlan);

        /**
         * VLAN ID
         */
        vlan_id_t m_vlan;

        /**
         * Refernece conter lock on the parent
         */
        const std::shared_ptr<Interface> m_parent;

        /**
         * A map of all sub-interfaces key against the sub-interface's name
         */
        static InstDB<const std::string, SubInterface> m_db;
   };
};
#endif
