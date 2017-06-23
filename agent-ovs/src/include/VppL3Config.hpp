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
#include "VppInstDB.hpp"
#include "VppInterface.hpp"
#include "VppSubInterface.hpp"

extern "C"
{
    #include "interface.api.vapi.h"
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
        L3Config(const SubInterface &itf,
                 const Route::prefix_t &pfx);
        ~L3Config();
        L3Config(const L3Config& o);

        /**
         * Debug print function
         */
        std::string to_string() const;

        /**
         * A functor class that binds the L3 config to the interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    const Route::prefix_t &pfx);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const BindCmd&i) const;
        private:
            const handle_t &m_itf;
            const Route::prefix_t m_pfx;
        };

        /**
         * A cmd class that Unbinds L3 Config from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
            UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      const Route::prefix_t &pfx);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const UnbindCmd&i) const;
        private:
            const handle_t m_itf;
            const Route::prefix_t m_pfx;
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
