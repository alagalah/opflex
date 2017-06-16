/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_OBJECT_DB_H__
#define __VPP_OBJECT_DB_H__

#include <map>

#include "VppObject.hpp"

namespace VPP
{
    /**
     * The DataBase of all VppObjects.
     */
    class ObjectDB
    {
    public:
        typedef std::map<const Object*, Object*>::iterator iterator;
        typedef std::pair<const Object*, Object*> entry;

        static iterator begin();
        static iterator end();
    
    private:
        /**
         * no instantiations
         */
        ObjectDB();
        ~ObjectDB();

        static void add(Object *obj);
        static void remove(Object *obj);

        friend class Object;

        /**
         * A map of all the objects
         */
        static std::map<const Object*, Object*> m_objs;
    };
};

#endif
