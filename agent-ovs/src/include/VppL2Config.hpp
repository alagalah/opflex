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
#include "VppInstDB.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppVxlanTunnel.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class L2Config: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        L2Config(const Interface &itf,
                 const BridgeDomain &bd);
        ~L2Config();
        L2Config(const L2Config& o);

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * A functor class that binds L2 configuration to an interface
         */
        class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
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
            const handle_t m_itf;
            uint32_t m_bd;
            bool m_is_bvi;
        };

        /**
         * A cmd class that Unbinds L2 configuration from an interface
         */
        class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
        {
        public:
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
            const handle_t m_itf;
            uint32_t m_bd;
            bool m_is_bvi;
        };

    private:
        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const L2Config &obj);

        static std::shared_ptr<L2Config> find_or_add(const L2Config &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

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
        static InstDB<const handle_t, L2Config> m_db;
    };
};

#endif
