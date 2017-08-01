/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ARP_PROXY_CONFIG_H__
#define __VPP_ARP_PROXY_CONFIG_H__

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
#include "VppInspect.hpp"

namespace VPP
{
    /**
     * A representation of LLDP client configuration on an interface
     */
    class ArpProxyConfig: public Object
    {
    public:
        /**
         * Dependency level 'Binding'
         */
        const static dependency_t dependency_value = dependency_t::GLOBAL;

        /**
         * Key type
         */
        typedef std::pair<boost::asio::ip::address_v4,
                          boost::asio::ip::address_v4> key_t;

        /**
         * Construct a new object matching the desried state
         */
        ArpProxyConfig(const boost::asio::ip::address_v4 &low,
                       const boost::asio::ip::address_v4 &high);

        /**
         * Copy Constructor
         */
        ArpProxyConfig(const ArpProxyConfig& o);

        /**
         * Destructor
         */
        ~ArpProxyConfig();

        /**
         * Return the 'singular' of the LLDP config that matches this object
         */
        std::shared_ptr<ArpProxyConfig> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all LLDP configs into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * A command class that adds the ARP Proxy config
         */
        class ConfigCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            ConfigCmd(HW::Item<bool> &item,
                      const boost::asio::ip::address_v4 &lo,
                      const boost::asio::ip::address_v4 &high);

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
            bool operator==(const ConfigCmd&i) const;
        private:
            /**
             * Address range
             */
            const boost::asio::ip::address_v4 m_low;
            const boost::asio::ip::address_v4 m_high;
        };

        /**
         * A cmd class that Unconfigs ArpProxy Config from an interface
         */
        class UnconfigCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            UnconfigCmd(HW::Item<bool> &item,
                      const boost::asio::ip::address_v4 &lo,
                      const boost::asio::ip::address_v4 &hig);

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
            bool operator==(const UnconfigCmd&i) const;
        private:
            /**
             * Address range
             */
            const boost::asio::ip::address_v4 m_low;
            const boost::asio::ip::address_v4 m_high;
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
        void update(const ArpProxyConfig &obj);

        /**
         * Find or add LLDP config to the OM
         */
        static std::shared_ptr<ArpProxyConfig> find_or_add(const ArpProxyConfig &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<ArpProxyConfig::key_t, ArpProxyConfig>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * Address range
         */
        const boost::asio::ip::address_v4 m_low;
        const boost::asio::ip::address_v4 m_high;

        /**
         * A map of all ArpProxy configs keyed against the interface.
         */
        static SingularDB<ArpProxyConfig::key_t, ArpProxyConfig> m_db;

        /**
         * HW configuration for the config. The bool representing the
         * do/don't configured/unconfigured.
         */
        HW::Item<bool> m_config;
    };

    std::ostream & operator<<(std::ostream &os, const ArpProxyConfig::key_t &key);
};

#endif
