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
                     admin_state_t state,
                     Route::prefix_t prefix);
        ~TapInterface();
        TapInterface(const TapInterface& o);

        /**
         * Return the matching 'instance' of the TAP interface
         */
        std::shared_ptr<TapInterface> instance() const;

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

    private:
        /**
         * Construct with a handle
         */
        TapInterface(const handle_t &hdl,
                     const std::string &name,
                     admin_state_t state,
                     Route::prefix_t prefix);

        /**
         * Interface is a friend so it can construct with handles
         */
        friend class Interface;

        /**
         * Return the matching 'instance' of the sub-interface
         *  over-ride from the base class
         */
        std::shared_ptr<Interface> instance_i() const;

        /**
         * Virtual functions to construct an interface create commands.
         */
        virtual Cmd* mk_create_cmd();

        /**
         * Virtual functions to construct an interface delete commands.
         */
        virtual Cmd* mk_delete_cmd();

        /*
         * It's the VPP::OM class that call instance()
         */
        friend class VPP::OM;

        /**
         * Ip Prefix
         */
        Route::prefix_t m_prefix;
    };

}

#endif
