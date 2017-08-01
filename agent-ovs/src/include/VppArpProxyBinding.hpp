/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ARP_PROXY_BINDING_H__
#define __VPP_ARP_PROXY_BINDING_H__

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppDumpCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppInterface.hpp"
#include "VppArpProxyConfig.hpp"
#include "VppInspect.hpp"

extern "C"
{
    #include "lldp.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of LLDP client configuration on an interface
     */
    class ArpProxyBinding: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        ArpProxyBinding(const Interface &itf,
                        const ArpProxyConfig &proxy_cfg);

        /**
         * Copy Constructor
         */
        ArpProxyBinding(const ArpProxyBinding& o);

        /**
         * Destructor
         */
        ~ArpProxyBinding();


        /**
         * Return the 'singular' of the LLDP binding that matches this object
         */
        std::shared_ptr<ArpProxyBinding> singular() const;

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
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            BindCmd(HW::Item<bool> &item,
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
            bool operator==(const BindCmd&i) const;
        private:
            /**
             * Reference to the HW::Item of the interface to bind
             */
            const handle_t &m_itf;
        };

        /**
         * A cmd class that Unbinds ArpProxy Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
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
        void update(const ArpProxyBinding &obj);

        /**
         * Find or add LLDP binding to the OM
         */
        static std::shared_ptr<ArpProxyBinding> find_or_add(const ArpProxyBinding &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<Interface::key_type, ArpProxyBinding>;

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
         * A reference counting pointer to the prxy config.
         */
        const std::shared_ptr<ArpProxyConfig> m_arp_proxy_cfg;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all ArpProxy bindings keyed against the interface.
         */
        static SingularDB<Interface::key_type, ArpProxyBinding> m_db;
    };
};

#endif
