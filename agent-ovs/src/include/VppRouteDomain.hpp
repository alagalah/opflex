/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ROUTE_DOMAIN_H__
#define __VPP_ROUTE_DOMAIN_H__

#include <string>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppSingularDB.hpp"
#include "VppRoute.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class RouteDomain: public Object
    {
    public:
        /**
         * Dependency level domain
         */
        const static dependency_t dependency_value = dependency_t::FORWARDING_DOMAIN;

        /**
         * Construct a new object matching the desried state
         */
        RouteDomain(Route::table_id_t id);
        /**
         * Copy Constructor
         */
        RouteDomain(const RouteDomain& o);
        /**
         * Destructor
         */
        ~RouteDomain();

        /**
         * Return the matching 'singular instance'
         */
        std::shared_ptr<RouteDomain> singular() const;

        /**
         * Debug print function
         */
        std::string to_string() const;

        /**
         * Get the table ID
         */
        Route::table_id_t table_id() const;

        /**
         * Find the instnace of the route domain in the OM
         */
        static std::shared_ptr<RouteDomain> find(const RouteDomain &temp);

        /**
         * Dump all route-doamin into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const RouteDomain &obj);

        /**
         * Find or add the instnace of the route domain in the OM
         */
        static std::shared_ptr<RouteDomain> find_or_add(const RouteDomain &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<Route::table_id_t, RouteDomain>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * VPP understands Table-IDs not table names.
         *  The table IDs for V4 and V6 are the same.
         */
        Route::table_id_t m_table_id;

        /**
         * A map of all interfaces key against the interface's name
         */
        static SingularDB<Route::table_id_t, RouteDomain> m_db;
    };
};

#endif
