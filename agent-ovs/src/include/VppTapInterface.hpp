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
     * A tap-interface. e.g. a tap interface
     */
    class TapInterface: public Interface
    {

    public:
        
        TapInterface(const std::string &name,
                         type_t type,
                         admin_state_t state,
                         Route::prefix_t prefix);
        ~TapInterface();
        TapInterface(const TapInterface& o);

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name,
                      Route::prefix_t &prefix);

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
            const std::string &m_name;
            Route::prefix_t &m_prefix;
        };

        /**
         * 
         */
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
        {
        public:
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

        /**
         * The the instance of the Interface in the Object-Model
         */
        static std::shared_ptr<TapInterface> find(const TapInterface &temp);        

    private:

        /**
         * Virtual functions to construct an interface create/delete commands.
         */
        virtual Cmd* mk_create_cmd();
        virtual Cmd* mk_delete_cmd();

        static std::shared_ptr<TapInterface> find_or_add(const TapInterface &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
         * Ip Prefix
         */
        Route::prefix_t m_prefix;

        /**
         * A map of all tap-interfaces key against the tap-interface's name
         */
        static InstDB<const std::string, TapInterface> m_db;
    };

}

#endif
