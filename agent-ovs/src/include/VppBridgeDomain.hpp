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
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
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
        BridgeDomain(uint32_t id);
        /**
         * Copy Constructor
         */
        BridgeDomain(const BridgeDomain& o);
        /**
         * Destructor
         */
        ~BridgeDomain();

        /**
         * Return the matchin 'instance' of the bridge-domain
         */
        std::shared_ptr<BridgeDomain> instance() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string(void) const;

        /**
         * Return VPP's handle for this obejct
         */
        uint32_t id() const;

        /**
         * Static function to find the BridgeDomain in the model
         */
        static std::shared_ptr<BridgeDomain> find(const BridgeDomain &temp);

        /**
         * A command class that creates an Bridge-Domain
         */
        class CreateCmd: public RpcCmd<HW::Item<uint32_t>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            CreateCmd(HW::Item<uint32_t> &item);

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
            bool operator==(const CreateCmd&i) const;
        };

        /**
         * A cmd class that Delete an Bridge-Domain
         */
        class DeleteCmd: public RpcCmd<HW::Item<uint32_t>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            DeleteCmd(HW::Item<uint32_t> &item);

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
            bool operator==(const DeleteCmd&i) const;
        };

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const BridgeDomain &obj);

        /**
         * Find or add an instance of a Bridge-Domain in the Object Model
         */
        static std::shared_ptr<BridgeDomain> find_or_add(const BridgeDomain &temp);

        /*
         * It's the VPP::OM class that calls instance()
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);
    
        /**
         * The ID we assign to this BD and the HW result in VPP
         */
        HW::Item<uint32_t> m_id;

        /**
         * A map of all interfaces key against the interface's name
         */
        static InstDB<uint32_t, BridgeDomain> m_db;
    };
};

#endif
