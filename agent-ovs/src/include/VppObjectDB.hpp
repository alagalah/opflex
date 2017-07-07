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
        /**
         * Iterator typedef
         */
        typedef std::map<const Object*, Object*>::iterator iterator;

        /**
         * DB entry type def
         */
        typedef std::pair<const Object*, Object*> entry;

        /**
         * Get iterator begin
         */
        static iterator begin();

        /**
         * get iterator end
         */
        static iterator end();
    
    private:
        /**
         * no instantiations
         */
        ObjectDB();

        /**
         * Add an object to the DB
         */
        static void add(Object *obj);

        /**
         * remove an object from the DB
         */
        static void remove(Object *obj);

        /**
         * Only Objects can add and removes themselves from the DB
         */
        friend class Object;

        /**
         * A map of all the objects
         */
        static std::map<const Object*, Object*> m_objs;
    };
};

#endif
