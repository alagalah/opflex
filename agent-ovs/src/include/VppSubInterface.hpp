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
        typedef uint16_t vlan_id_t;

    public:
        /**
         * Construct a new object matching the desried state
         */
        SubInterface(const Interface &parent,
                     admin_state_t state,
                     vlan_id_t vlan);
        /**
         * Destructor
         */
        ~SubInterface();
        /**
         * Copy Constructor
         */
        SubInterface(const SubInterface& o);

        /**
         * Return the matching 'singular instance' of the sub-interface
         */
        std::shared_ptr<SubInterface> singular() const;

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public Interface::CreateCmd<vapi::Create_vlan_subif>
        {
        public:
            /**
             * Cstrunctor taking the reference to the parent
             * and the sub-interface's VLAN
             */
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name,
                      const handle_t &parent,
                      uint16_t vlan);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const CreateCmd&i) const;

        private:
            /**
             * Refernece to the parents handle
             */
            const handle_t &m_parent;

            /**
             * The VLAN of the sub-interface
             */
            uint16_t m_vlan;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public Interface::DeleteCmd<vapi::Delete_subif>
        {
        public:
            /**
             * Constructor
             */
            DeleteCmd(HW::Item<handle_t> &item);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const DeleteCmd&i) const;
        };

    private:
        /**
         * Construct with handle
         */
        SubInterface(const handle_t &handle,
                     const Interface &parent,
                     admin_state_t state,
                     vlan_id_t vlan);
        /**
         * The interface class can construct Interfaces with handles
         */
        friend class Interface;

        /**
         * Return the matching 'instance' of the sub-interface
         *  over-ride from the base class
         */
        std::shared_ptr<Interface> singular_i() const;

        /**
         * Virtual functions to construct an interface create commands.
         */
        virtual std::queue<Cmd*> & mk_create_cmd(std::queue<Cmd*> &cmds);

        /**
         * Virtual functions to construct an interface delete commands.
         */
        virtual std::queue<Cmd*> & mk_delete_cmd(std::queue<Cmd*> &cmds);

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
   };
};
#endif
