/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_OM_H__
#define __VPP_OM_H__

#include <cassert>
#include <queue>
#include <algorithm>
#include <memory>
#include <list>

#include "VppKeyDB.hpp"
#include "VppHW.hpp"

namespace VPP {    
    /**
     * The interface to writing objects into VPP OM.
     */
    class OM {
    public:
        /**
         * Init
         */
        static void init();

        /**
         * Mark all state owned by this key as stale
         */
        static void mark(const KEY &key);

        /**
         * Sweep all the key's objects that are stale
         */
        static void sweep(const KEY &key);

        /**
         * Make the State in VPP reflect tha expressed desired state.
         *  But don't call the HW - use this whilst processing dumped
         *  data from HW
         */
        template <typename OBJ>
        static rc_t commit(const KEY &key, const OBJ &obj)
        {
            rc_t rc = rc_t::OK;

            HW::disable();
            rc = VPP::OM::write(key, obj);
            HW::enable();

            return (rc);
        }

        /**
         * Make the State in VPP reflect tha expressed desired state.
         *  After processing all the objects in the queue, in FIFO order,
         *  any remaining state owned by the KEY is purged.
         */
        template <typename OBJ>
        static rc_t write(const KEY &key, const OBJ &obj)
        {
            rc_t rc = rc_t::OK;

            /*
             * Find the instance another owner may have created.
             * this always returns something.
             */
            std::shared_ptr<OBJ> inst(obj.instance());

            /*
             * Update the existing object with with the new desired state
             */
            inst->update(obj);

            /*
             * Find if the object already stored on behalf of this key.
             * and mark them stale
             */
            VPP::ObjectRefList& objs = m_db->find(key);

            /*
             * Iterate through this list to find a matchin' object
             * to the one requested.
             */
            auto match_ptr = [inst](const ObjectRef &oref)
            {
                return (inst == oref.obj());
            };
            auto it = std::find_if(objs.begin(), objs.end(), match_ptr);
            
            if (it != objs.end())
            {
                /*
                 * yes, this key already owns this object.
                 */
                it->clear();
            }
            else
            {
                /*
                 * Add the instance to the owners list
                 */
                objs.insert(ObjectRef(inst));
            }

            return (HW::write());
        }

        /**
         * Remove all object in the OM referenced by the key
         */
        static void remove(const KEY &key);

        /**
         * Print each of the object in the DB into the stream provided
         */
        static void dump(const KEY & key, std::ostream &os);

        /**
         * Print each of the KEYS
         */
        static void dump(std::ostream &os);

    private:
        /**
         * Database of object state created for each key
         */
        static KeyDB *m_db;
    };
}
#endif
