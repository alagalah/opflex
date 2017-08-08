/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_LLDP_GLOBAL_H__
#define __VPP_LLDP_GLOBAL_H__

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
     * A representation of LLDP global configuration
     */
    class LldpGlobal: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        LldpGlobal(const std::string &system_name,
                   uint32_t tx_hold,
                   uint32_t tx_interval);

        /**
         * Copy Constructor
         */
        LldpGlobal(const LldpGlobal& o);

        /**
         * Destructor
         */
        ~LldpGlobal();

        /**
         * Return the 'singular' of the LLDP global that matches this object
         */
        std::shared_ptr<LldpGlobal> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all LLDP globals into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * A command class that binds the LLDP global to the interface
         */
        class ConfigCmd: public RpcCmd<HW::Item<bool>, rc_t, vapi::Lldp_config>
        {
        public:
            /**
             * Constructor
             */
            ConfigCmd(HW::Item<bool> &item,
                      const std::string &system_name,
                      uint32_t tx_hold,
                      uint32_t tx_interval);

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
             * The system name
             */
            const std::string m_system_name;

            /**
             * TX timer configs
             */
            uint32_t m_tx_hold;
            uint32_t m_tx_interval;
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
        void update(const LldpGlobal &obj);

        /**
         * Find or add LLDP global to the OM
         */
        static std::shared_ptr<LldpGlobal> find_or_add(const LldpGlobal &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<Interface::key_type, LldpGlobal>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * The system name
         */
        const std::string m_system_name;

        /**
         * TX timer configs
         */
        uint32_t m_tx_hold;
        uint32_t m_tx_interval;

        /**
         * HW globaluration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all Lldp globals keyed against the system name.
         *  there needs to be some sort of key, that will do.
         */
        static SingularDB<std::string, LldpGlobal> m_db;
    };
};

#endif
