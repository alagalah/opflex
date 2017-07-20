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
#include <set>

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
         * populate the OM with state read from HW.
         */
        static void populate(const KeyDB::key_t &key);

        /**
         * Mark all state owned by this key as stale
         */
        static void mark(const KeyDB::key_t &key);

        /**
         * Sweep all the key's objects that are stale
         */
        static void sweep(const KeyDB::key_t &key);

        /**
         * Replay all of the objects to HW.
         */
        static void replay(void);

        /**
         * Make the State in VPP reflect tha expressed desired state.
         *  But don't call the HW - use this whilst processing dumped
         *  data from HW
         */
        template <typename OBJ>
        static rc_t commit(const KeyDB::key_t &key, const OBJ &obj)
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
         *  any remaining state owned by the KeyDB::key_t is purged.
         * This is a template function so the object's update() function is
         * always called with the deirved type.
         */
        template <typename OBJ>
        static rc_t write(const KeyDB::key_t &key, const OBJ &obj)
        {
            rc_t rc = rc_t::OK;

            /*
             * Find the singular instance another owner may have created.
             * this always returns something.
             */
            std::shared_ptr<OBJ> inst = obj.singular();

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
                 * Add the singular instance to the owners list
                 */
                objs.insert(ObjectRef(inst));
            }

            return (HW::write());
        }

        /**
         * Remove all object in the OM referenced by the key
         */
        static void remove(const KeyDB::key_t &key);

        /**
         * Print each of the object in the DB into the stream provided
         */
        static void dump(const KeyDB::key_t & key, std::ostream &os);

        /**
         * Print each of the KEYS
         */
        static void dump(std::ostream &os);

        /**
         * Class definition for listeners to OM events
         */
        class Listener
        {
        public:
            Listener() = default;
            virtual ~Listener() = default;

            /**
             * Handle a populate event
             */
            virtual void handle_populate(const KeyDB::key_t & key) = 0;

            /**
             * Handle a replay event
             */
            virtual void handle_replay() = 0;

            /**
             * Get the sortable Id of the listener
             */
            virtual dependency_t order() const = 0;

            /**
             * less than operator for set sorting
             */
            bool operator<(const Listener &listener) const
            {
                return (order() < listener.order());
            }
        };

        /**
         * Register a listener of events
         */
        static bool register_listener(Listener *listener);

    private:
        /**
         * Database of object state created for each key
         */
        static KeyDB *m_db;

        /**
         * Comparator to keep thte pointers to Listeners in sorted order
         */
        struct listener_comparator_t
        {
            bool operator() (const Listener *l1, const Listener *l2) const
            {
                return (l1->order() < l2->order());
            }
        };

        /**
         * convenient typedef for the sorted set of listeners
         */
        typedef std::multiset<Listener*, listener_comparator_t> ListenerList;

        /**
         * The listeners for events
         */
        static std::unique_ptr<ListenerList> m_listeners;
    };
}
#endif
