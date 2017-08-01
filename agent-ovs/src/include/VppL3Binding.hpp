/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_L3_BINDING_H__
#define __VPP_L3_BINDING_H__

#include <string>
#include <map>
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

extern "C"
{
    #include "ip.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of L3 configuration on an interface
     */
    class L3Binding: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        L3Binding(const Interface &itf,
                  const Route::prefix_t &pfx);
        
        /**
         * Copy Constructor
         */
        L3Binding(const L3Binding& o);

        /**
         * Destructor
         */
        ~L3Binding();

        /**
         * Return the 'singular instance' of the L3-Config that matches this object
         */
        std::shared_ptr<L3Binding> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Return the prefix associated with this L3config
         */
        const Route::prefix_t& prefix() const;

        /**
         * Dump all L3Bindings into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * The key type for L3Bindings
         */
        typedef std::pair<Interface::key_type, Route::prefix_t> key_type_t;

        /**
         * Find an singular instance in the DB for the interface passed
         */
        static std::deque<std::shared_ptr<L3Binding>> find(const Interface &i);

        /**
         * A functor class that binds the L3 config to the interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    const Route::prefix_t &pfx);

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
             * Reference to the interface to bind to
             */
            const handle_t &m_itf;

            /**
             * The prefix to bind
             */
            const Route::prefix_t m_pfx;
        };

        /**
         * A cmd class that Unbinds L3 Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
           UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      const Route::prefix_t &pfx);

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
             * Reference to the interface to unbind fomr
             */
            const handle_t m_itf;

            /**
             * The prefix to unbind
             */
           const Route::prefix_t m_pfx;
        };

        /**
         * A cmd class that Dumps all the IPv4 L3 configs
         */
        class DumpV4Cmd: public DumpCmd<vapi_payload_ip_address_details>
        {
        public:
            /**
             * Constructor
             */
            DumpV4Cmd(const handle_t &itf);
            DumpV4Cmd(const DumpV4Cmd &d);

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
            bool operator==(const DumpV4Cmd&i) const;
        private:
            /**
             * HW reutrn code
             */
            HW::Item<bool> item;

            /**
             * The interface to get the addresses for
             */
            const handle_t m_itf;
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
        void update(const L3Binding &obj);

        /**
         * Find or add the singular instance in the DB
         */
        static std::shared_ptr<L3Binding> find_or_add(const L3Binding &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
        e* It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<key_type_t, L3Binding>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * A reference counting pointer the interface that this L3 layer
         * represents. By holding the reference here, we can guarantee that
         * this object will outlive the interface
         */
        const std::shared_ptr<Interface> m_itf;
    
        /**
         * The prefix for this L3 configuration
         */
        Route::prefix_t m_pfx;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all L3 configs keyed against a combination of the interface
         * and subnet's keys.
         */
        static SingularDB<key_type_t, L3Binding> m_db;
    };

    /**
     * Ostream output for the key
     */
    std::ostream &operator<<(std::ostream &os, const L3Binding::key_type_t &key);
};

#endif
