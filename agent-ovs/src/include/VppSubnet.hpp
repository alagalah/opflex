/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_SUBNET_H__
#define __VPP_SUBNET_H__

#include <string>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppRouteDomain.hpp"
#include "VppOM.hpp"
#include "VppInstDB.hpp"
#include "VppRoute.hpp"

#include <boost/asio/ip/address.hpp>

namespace VPP
{
    /**
     * A representation of a Subnet. This exists in one or more RouteDomains
     */
    class Subnet: public Object
    {
    public:
        /**
         * The key type for subnets
         */
        typedef std::string key_tpe;

        /**
         * Construct a new object matching the desried state
         */
        Subnet(const Route::prefix_t &p);

        /**
         * Copy Constructor
         */
        Subnet(const Subnet& o);

        /**
         * Destructor
         */
        ~Subnet();

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const Subnet &obj);

        /**
         * Find or add a subnet instance in the OM
         */
        static std::shared_ptr<Subnet> find_or_add(const Subnet &temp);

        /**
         * Find a subnet instace in the OM
         */
        static std::shared_ptr<Subnet> find(const Subnet &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * The Subnet's prefix
         */
        Route::prefix_t m_prefix;

        /**
         * A map of all subnet's key against the prefix
         */
        static InstDB<const Route::prefix_t, Subnet> m_db;
    };
};

#endif
