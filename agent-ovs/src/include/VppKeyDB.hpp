/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_KEY_DB_H__
#define __VPP_KEY_DB_H__

#include <map>
#include <set>
#include <memory>

#include "VppObject.hpp"

typedef const std::string KEY;

namespace VPP
{
    /**
     * A convenitent typedef for set of objects owned.
     *  A set of shared pointers. This is how the reference counting
     *  of an object in the model it managed. Once all these shared ptr
     *  and hance references are gone, the object is deleted and any state
     *  in VPP is removed.
     */
    typedef std::set<ObjectRef> ObjectRefList;

    /**
     * A DB storing the objects that each owner owns.
     */
    class KeyDB
    {
    public:
        ObjectRefList& find(KEY &k);
        void flush(KEY &k);

    private:
        /**
         * A map of keys versus the object they reference
         */
        std::map<KEY, ObjectRefList> m_objs;
    };
};

#endif
