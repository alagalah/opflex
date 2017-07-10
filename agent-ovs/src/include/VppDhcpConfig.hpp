/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_DHCP_INTERFACE_H__
#define __VPP_DHCP_INTERFACE_H__

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

extern "C"
{
    #include "dhcp.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of DHCP client configuration on an interface
     */
    class DhcpConfig: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        DhcpConfig(const Interface &itf,
                   const std::string &hostname);
        /**
         * Copy Constructor
         */
        DhcpConfig(const DhcpConfig& o);
        /**
         * Destructor
         */
        ~DhcpConfig();


        /**
         * Return the 'singular' of the DHCP config that matches this object
         */
        std::shared_ptr<DhcpConfig> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * A command class that binds the DHCP config to the interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    const std::string &hostname);

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
             * The DHCP client's hostname
             */
            const std::string m_hostname;
        };

        /**
         * A cmd class that Unbinds Dhcp Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      const std::string &hostname);

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

            /**
             * The DHCP client's hostname
             */
            const std::string m_hostname;
        };

        /**
         * Forward declartion of the Event Command
         */
        class EventsCmd;

        /**
         * A class that listens to DHCP Events
         */
        class EventListener
        {
        public:
            /**
             * Constructor
             */
            EventListener();

            /**
             * Listener's virtual function invoked when a DHCP event is
             * available to read
             */
            virtual void handle_dhcp_event(EventsCmd *cmd) = 0;

            /**
             * Return the HW::Item associated with this command
             */            
            HW::Item<bool> & status();
        protected:
            /**
             * The HW::Item associated with this command
             */
            HW::Item<bool> m_status;
        };

        /**
         * A functor class represents our desire to recieve interface events
         */
        class EventsCmd: public RpcCmd<HW::Item<bool>, rc_t>,
                         public EventCmd<vapi_payload_dhcp_compl_event>
        {
        public:
            /**
             * Constructor
             */
            EventsCmd(EventListener &el);

            /**
             * Issue the command to VPP/HW - subscribe to DHCP events
             */
            rc_t issue(Connection &con);

            /**
             * Retire the command - unsubscribe
             */
            void retire();
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const EventsCmd&i) const;

            /**
             * called in the VAPI RX thread when data is available.
             */
            void notify(vapi_payload_dhcp_compl_event *data);
        private:
            /**
             * The listner of this command
             */
            EventListener & m_listener;
        };


    private:
        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const DhcpConfig &obj);

        /**
         * Find or add DHCP config to the OM
         */
        static std::shared_ptr<DhcpConfig> find_or_add(const DhcpConfig &temp);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * A reference counting pointer to the interface on which DHCP config
         * resides. By holding the reference here, we can guarantee that
         * this object will outlive the interface
         */
        const std::shared_ptr<Interface> m_itf;
    
        /**
         * The hostname in the DHCP configuration
         */
        const std::string m_hostname;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::Item<bool> m_binding;

        /**
         * A map of all Dhcp configs keyed against the interface.
         */
        static SingularDB<Interface::key_type, DhcpConfig> m_db;
    };
};

#endif
