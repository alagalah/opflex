/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_IP_UNNUMBERED_H__
#define __VPP_IP_UNNUMBERED_H__

#include <string>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppInterface.hpp"
#include "VppInspect.hpp"

namespace VPP
{
    /**
     * A representation of IP unnumbered configuration on an interface
     */
    class IpUnnumbered: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         *
         * @param itf - The Interface with no IP address
         * @param l3_itf - The interface that has the IP address we wish to share.
         */
        IpUnnumbered(const Interface &itf,
                     const Interface &l3_itf);
        
        /**
         * Copy Constructor
         */
        IpUnnumbered(const IpUnnumbered& o);

        /**
         * Destructor
         */
        ~IpUnnumbered();

        /**
         * Return the 'singular instance' of the L3-Config that matches this object
         */
        std::shared_ptr<IpUnnumbered> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all IpUnnumbereds into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * The key type for IpUnnumbereds
         */
        typedef Interface::key_type key_t;

        /**
         * Find an singular instance in the DB for the interface passed
         */
        static std::shared_ptr<IpUnnumbered> find(const Interface &i);

        /**
         * A command class that configures the IP unnumbered
         */
        class ConfigCmd: public RpcCmd<HW::Item<bool>, rc_t,
                                       vapi::Sw_interface_set_unnumbered>
        {
        public:
            /**
             * Constructor
             */
            ConfigCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      const handle_t &l3_itf);

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
             * Reference to the interface for which the address is required
             */
            const handle_t &m_itf;
            /**
             * Reference to the interface which has an address
             */
            const handle_t &m_l3_itf;
        };

        /**
         * A cmd class that Unconfigs L3 Config from an interface
         */
        class UnconfigCmd: public RpcCmd<HW::Item<bool>, rc_t,
                                         vapi::Sw_interface_set_unnumbered>
        {
        public:
            /**
             * Constructor
             */
           UnconfigCmd(HW::Item<bool> &item,
                       const handle_t &itf,
                       const handle_t &l3_itf);

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
             * Reference to the interface for which the address is required
             */
            const handle_t &m_itf;
            /**
             * Reference to the interface which has an address
             */
            const handle_t &m_l3_itf;
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
        void update(const IpUnnumbered &obj);

        /**
         * Find or add the singular instance in the DB
         */
        static std::shared_ptr<IpUnnumbered> find_or_add(const IpUnnumbered &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
        e* It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<key_t, IpUnnumbered>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * A reference counting pointer the interface that requires an address.
         */
        const std::shared_ptr<Interface> m_itf;
        /**
         * A reference counting pointer the interface that has an address.
         */
        const std::shared_ptr<Interface> m_l3_itf;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_config;

        /**
         * A map of all L3 configs keyed against a combination of the interface
         * and subnet's keys.
         */
        static SingularDB<key_t, IpUnnumbered> m_db;
    };
};

#endif
