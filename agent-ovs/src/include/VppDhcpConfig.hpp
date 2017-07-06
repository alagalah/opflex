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
#include "VppInstDB.hpp"
#include "VppInterface.hpp"
#include "VppSubInterface.hpp"

extern "C"
{
    #include "dhcp.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of Dhcp configuration on an interface
     */
    class DhcpConfig: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        DhcpConfig(const Interface &itf,
                   const std::string &hostname);
        ~DhcpConfig();
        DhcpConfig(const DhcpConfig& o);

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * A functor class that binds the Dhcp config to the interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
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
            const handle_t &m_itf;
            const std::string m_hostname;
        };

        /**
         * A cmd class that Unbinds Dhcp Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
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
            const handle_t m_itf;
            const std::string m_hostname;
        };

        /**
         * A class that listens to Interface Events
         */
        class EventsCmd;
        class EventListener
        {
        public:
            EventListener();
            virtual void handle_dhcp_event(EventsCmd *cmd) = 0;

            HW::Item<bool> & status();
        protected:
            HW::Item<bool> m_status;
        };

        /**
         * A functor class represents our desire to recieve interface events
         */
        class EventsCmd: public RpcCmd<HW::Item<bool>, rc_t>,
                         public EventCmd<vapi_payload_dhcp_compl_event>
        {
        public:
            EventsCmd(EventListener &el);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);
            void retire();
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const EventsCmd&i) const;

            void notify(vapi_payload_dhcp_compl_event *data);
        private:
            EventListener & m_listener;
        };


    private:
        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const DhcpConfig &obj);

        static std::shared_ptr<DhcpConfig> find_or_add(const DhcpConfig &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
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
        static InstDB<Interface::key_type, DhcpConfig> m_db;
    };
};

#endif
