/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_OBJECT_H__
#define __VPP_OBJECT_H__

#include <string>
#include <memory>

#include "Vpp.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class Object
    {
    public:
        /**
         * convert to string format for debug purposes
         */
        virtual std::string to_string() const = 0;

        /**
         * Sweep/reap the object if still stale
         */
        virtual void sweep(void) = 0;

    protected:
        /**
         * Constructable by derived classes only
         */
        Object();
        virtual ~Object();

    private:
        /**
         * note we are not maintaining dependencies back to the
         * keys. i.e. this object does not know all the keys that
         * refer to it.
         */
    };

    /**
     * object state
     */
    enum obj_state_t
    {
        OBJECT_STATE_NONE = 0,
        /**
         * indicates the object is stale. This flag is set
         * when a new epoch is declared. the flag is cleared
         * when the object is updated in the new epoch. If the
         * flag is still set after convergence is declared then
         * the object is deleted
         */
        OBJECT_STATE_STALE,
    };

    /**
     * A represenation of a reference to a VPP object.
     *  the reference counting is held through the use of shared pointers.
     * We also maintain the state of the object ready for mark n' sweep.
     */
    class ObjectRef
    {
    public:
        ObjectRef(std::shared_ptr<Object> obj);

        bool operator<(const ObjectRef &other) const;
        std::shared_ptr<Object> obj() const;
        void mark()  const;
        void clear() const;
        bool stale() const;

    private:
        std::shared_ptr<Object> m_obj;

        /**
         * Not part of the key (in the set) so we can change it
         * whilst iterating
         */
        mutable obj_state_t m_state;
    };

    std::ostream& operator<<(std::ostream &os, const Object& o);
    
};
#endif
