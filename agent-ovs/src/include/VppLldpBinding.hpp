/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_LLDP_BINDING_H__
#define __VPP_LLDP_BINDING_H__

#include <string>
#include <map>
#include <vector>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppDumpCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppInterface.hpp"
#include "VppSubInterface.hpp"
#include "VppInspect.hpp"

#include <vapi/lldp.api.vapi.hpp>

namespace VPP
{
    /**
     * A representation of LLDP client configuration on an interface
     */
    class LldpBinding: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        LldpBinding(const Interface &itf,
                   const std::string &hostname);

        /**
         * Copy Constructor
         */
        LldpBinding(const LldpBinding& o);
        /**
         * Destructor
         */
        ~LldpBinding();


        /**
         * Return the 'singular' of the LLDP binding that matches this object
         */
        std::shared_ptr<LldpBinding> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all LLDP bindings into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * A command class that binds the LLDP config to the interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t,
                                     vapi::Sw_interface_set_lldp>
        {
        public:
            /**
             * Constructor
             */
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    const std::string &port_desc);

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
            bool operator==(const BindCmd&i) const;
        private:
            /**
             * Reference to the HW::Item of the interface to bind
             */
            const handle_t &m_itf;

            /**
             * The LLDP client's hostname
             */
            const std::string m_port_desc;
        };

        /**
         * A cmd class that Unbinds Lldp Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t,
                                     vapi::Sw_interface_set_lldp>
        {
        public:
            /**
             * Constructor
             */
            UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf);

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
            bool operator==(const UnbindCmd&i) const;
        private:
            /**
             * Reference to the HW::Item of the interface to unbind
             */
            const handle_t &m_itf;
        };

    private:
        /**
         * Class definition for listeners to OM events
         */
        class EventHandler: public OM::Listener, public Inspect::CommandHandler
        {
        public:
            EventHandler();
            virtual ~EventHandler() = default;

            /**
             * Handle a populate event
             */
            void handle_populate(const KeyDB::key_t & key);

            /**
             * Handle a replay event
             */
            void handle_replay();

            /**
             * Show the object in the Singular DB
             */
            void show(std::ostream &os);

            /**
             * Get the sortable Id of the listener
             */
            dependency_t order() const;
        };

        /**
         * EventHandler to register with OM
         */
        static EventHandler m_evh;

        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const LldpBinding &obj);

        /**
         * Find or add LLDP binding to the OM
         */
        static std::shared_ptr<LldpBinding> find_or_add(const LldpBinding &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<Interface::key_type, LldpBinding>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * A reference counting pointer to the interface on which LLDP config
         * resides. By holding the reference here, we can guarantee that
         * this object will outlive the interface
         */
        const std::shared_ptr<Interface> m_itf;
    
        /**
         * The port-description in the LLDP configuration
         */
        const std::string m_port_desc;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all Lldp bindings keyed against the interface.
         */
        static SingularDB<Interface::key_type, LldpBinding> m_db;
    };
};

#endif
