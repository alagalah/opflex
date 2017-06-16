/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_BRIDGE_DOMAIN_H__
#define __VPP_BRIDGE_DOMAIN_H__

#include <string>
#include <map>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppCmd.hpp"
#include "VppInstDB.hpp"
#include "VppEnum.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class BridgeDomain: public Object
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        BridgeDomain(const std::string &name);
        ~BridgeDomain();
        BridgeDomain(const BridgeDomain& o);

        /**
         * Debug rpint function
         */
        std::string to_string(void);

        /**
         * Return VPP's handle for this obejct
         */
        const handle_t & handle() const;

        /**
         * Static function to find the BridgeDomain in the model
         */
        static std::shared_ptr<BridgeDomain> find(const BridgeDomain &temp);

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public CmdT<HW::Item<handle_t>>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name);

            rc_t exec();
            std::string to_string();

            bool operator==(const CreateCmd&i) const;
        private:
            const std::string &m_name;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public CmdT<HW::Item<handle_t>>
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item);

            rc_t exec();
            std::string to_string();

            bool operator==(const DeleteCmd&i) const;
        private:
        };

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const BridgeDomain &obj);

        static std::shared_ptr<BridgeDomain> find_or_add(const BridgeDomain &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * The BridgeDomain's name
         */
        const std::string m_name;
    
        /**
         * The SW interface handle VPP has asigned to the interface
         */
        HW::Item<handle_t> m_hdl;

        /**
         * A map of all interfaces key against the interface's name
         */
        static InstDB<const std::string, BridgeDomain> m_db;
    };
};

#endif
