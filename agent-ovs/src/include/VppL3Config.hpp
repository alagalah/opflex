/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_L3_INTERFACE_H__
#define __VPP_L3_INTERFACE_H__

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
    #include "ip.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of L3 configuration on an interface
     */
    class L3Config: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        L3Config(const Interface &itf,
                 const Route::prefix_t &pfx);

        /**
         * Construct a new object matching the desried state
         */
        L3Config(const SubInterface &itf,
                 const Route::prefix_t &pfx);
        
        /**
         * Copy Constructor
         */
        L3Config(const L3Config& o);

        /**
         * Destructor
         */
        ~L3Config();

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

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
        class DumpV4Cmd: public DumpCmd<vapi_payload_ip_fib_details>
        {
        public:
            /**
             * Constructor
             */
            DumpV4Cmd();

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
        };

    private:
        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const L3Config &obj);

        static std::shared_ptr<L3Config> find_or_add(const L3Config &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

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
        static InstDB<std::pair<Interface::key_type, Route::prefix_t>, L3Config> m_db;
    };
};

#endif
