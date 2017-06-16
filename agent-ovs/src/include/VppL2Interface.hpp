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
#include "VppCmd.hpp"
#include "VppInstDB.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class L2Interface: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        L2Interface(const Interface &itf,
                    const BridgeDomain &bd);
        ~L2Interface();
        L2Interface(const L2Interface& o);

        /**
         * Debug rpint function
         */
        std::string to_string(void);

        /**
         * A functor class that creates an interface
         */
        class BindCmd: public CmdT<HW::Item<bool>>
        {
        public:
            BindCmd(HW::Item<bool> &item,
                    const handle_t &itf,
                    const handle_t &bd,
                    bool is_bvi);

            rc_t exec();
            std::string to_string();

            bool operator==(const BindCmd&i) const;
        private:
            const handle_t m_itf;
            const handle_t m_bd;
            bool m_is_bvi;
        };

        /**
         * A cmd class that Unbind an interface
         */
        class UnbindCmd: public CmdT<HW::Item<bool>>
        {
        public:
            UnbindCmd(HW::Item<bool> &item,
                      const handle_t &itf,
                      const handle_t &bd,
                      bool is_bvi);

            rc_t exec();
            std::string to_string();

            bool operator==(const UnbindCmd&i) const;
        private:
            const handle_t m_itf;
            const handle_t m_bd;
            bool m_is_bvi;
        };

    private:
        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const L2Interface &obj);

        static std::shared_ptr<L2Interface> find_or_add(const L2Interface &temp);

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
        static InstDB<const handle_t, L2Interface> m_db;
    };
};

#endif
