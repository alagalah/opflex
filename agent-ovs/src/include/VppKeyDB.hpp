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

/**
 * In the opflex world each entity is known by a URI which can be converted
 * into a string. We use the string type, since it allows us to keep this VPP
 * specific code independent of opflex types. I might consider making this
 * a template parameter one day...
 */
typedef const std::string KEY;

namespace VPP
{
    /**
     * A convenitent typedef for set of objects owned.
     *  A set of shared pointers. This is how the reference counting
     *  of an object in the model it managed. Once all these shared ptr
     *  and hence references are gone, the object is deleted and any state
     *  in VPP is removed.
     */
    typedef std::set<ObjectRef> ObjectRefList;

    /**
     * A DB storing the objects that each owner/key owns.
     *  Each object is reference counter by each key that owns it. When
     * no more references exist the object is destroyed.
     */
    class KeyDB
    {
    public:
        /**
         * Find the objects owned by the key
         */
        ObjectRefList& find(const KEY &k);

        /**
         * flush, i.e. un-reference, all objects owned by the key
         */
        void flush(const KEY &k);

        /**
         * Print each of the object in the DB into the stream provided
         */
        void dump(const KEY & key, std::ostream &os)
        {
            ObjectRefList& orlist = find(key);

            for (auto entry : orlist)
            {
                os << "  " << entry.obj()->to_string() << std::endl;
            }
        }

    private:
        /**
         * A map of keys versus the object they reference
         */
        std::map<KEY, ObjectRefList> m_objs;
    };
};

#endif
