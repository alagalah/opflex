/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_L2_INTERFACE_H__
#define __VPP_L2_INTERFACE_H__

#include <string>
#include <map>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppVxlanTunnel.hpp"
#include "VppInspect.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class L2Binding: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        L2Binding(const Interface &itf,
                 const BridgeDomain &bd);

        /**
         * Copy Constructor
         */
        L2Binding(const L2Binding& o);

        /**
         * Destructor
         */
        ~L2Binding();

        /**
         * Return the 'singular instance' of the L2 config that matches this object
         */
        std::shared_ptr<L2Binding> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all L2Bindings into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * A functor class that binds L2 configuration to an interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    uint32_t bd,
                    bool is_bvi);

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
             * The interface to bind
             */
            const handle_t m_itf;

            /**
             * The bridge-domain to bind to
             */
            uint32_t m_bd;

            /**
             * Is it a BVI interface that is being bound
             */
            bool m_is_bvi;
        };

        /**
         * A cmd class that Unbinds L2 configuration from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      uint32_t bd,
                      bool is_bvi);

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
             * The interface to bind
             */
            const handle_t m_itf;

            /**
             * The bridge-domain to bind to
             */
            uint32_t m_bd;

            /**
             * Is it a BVI interface that is being bound
             */
            bool m_is_bvi;
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
        void update(const L2Binding &obj);

        /**
         * Find or Add the singular instance in the DB
         */
        static std::shared_ptr<L2Binding> find_or_add(const L2Binding &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<const handle_t, L2Binding>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * A reference counting pointer the interface that this L2 layer
         * represents. By holding the reference here, we can guarantee that
         * this object will outlive the interface
         */
        const std::shared_ptr<Interface> m_itf;
    
        /**
         * A reference counting pointer the Bridge-Domain that this L2
         * interface is bound to. By holding the reference here, we can
         * guarantee that this object will outlive the BD.
         */
        const std::shared_ptr<BridgeDomain> m_bd;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all L2 interfaces key against the interface's handle_t
         */
        static SingularDB<const handle_t, L2Binding> m_db;
    };
};

#endif
