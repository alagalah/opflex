/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_CONTROL_INTERFACE_H__
#define __VPP_CONTROL_INTERFACE_H__

#include "VppInterface.hpp"
#include "VppRoute.hpp"

namespace VPP
{

    /**
     * A control-interface. e.g. a tap interface
     */
    class ControlInterface: public Interface
    {

    public:
        
        ControlInterface(const std::string &name,
                         type_t type,
                         admin_state_t state,
                         Route::prefix_t prefix);
        ~ControlInterface();
        ControlInterface(const ControlInterface& o);

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      Interface &interface,       
                      Route::prefix_t &prefix);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const CreateCmd&i) const;
        private:
            Interface &m_interface;
            Route::prefix_t &m_prefix;
        };

        /**
         * 
         */
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const DeleteCmd&i) const;
        };

        /**
         * The the instance of the Interface in the Object-Model
         */
        static std::shared_ptr<ControlInterface> find(const ControlInterface &temp);        

    private:

        /**
         * Virtual functions to construct an interface create/delete commands.
         */
        virtual Cmd* mk_create_cmd();
        virtual Cmd* mk_delete_cmd();

        static std::shared_ptr<ControlInterface> find_or_add(const ControlInterface &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
         * Ip Prefix
         */
        Route::prefix_t m_prefix;

        /**
         * Interface
         */
        Interface m_interface;
         
        /**
         * A map of all control-interfaces key against the control-interface's name
         */
        static InstDB<const std::string, ControlInterface> m_db;
    };

}

#endif
