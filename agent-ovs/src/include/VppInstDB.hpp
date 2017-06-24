/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_INST_DB_H__
#define __VPP_INST_DB_H__

#include <memory>

#include "logging.h"

namespace VPP
{
    /**
     * A Database to store the unique instances of a single object type.
     * The instances are stored as weak pointers. So the DB does not own these
     * objects, they are owned by object in the KeyDB.
     */
    template <typename KEY, typename OBJ>
    class InstDB
    {
    public:
        InstDB()
        {
        }

        std::shared_ptr<OBJ> find_or_add(const KEY &key, const OBJ &obj)
        {
            auto search = m_map.find(key);

            if (search == m_map.end())
            {
                std::shared_ptr<OBJ> sp = std::make_shared<OBJ>(obj);

                m_map[key] = sp;

                LOG(ovsagent::INFO) << *sp;
                return (sp);
            }

            return (search->second.lock());
        }

        std::shared_ptr<OBJ> find(const KEY &key)
        {
            auto search = m_map.find(key);

            if (search == m_map.end())
            {
                std::shared_ptr<OBJ> sp(NULL);

                return (sp);
            }

            return (search->second.lock());
        }

        /*
         * Release the object from the DB store, if it's the one we have stored
         */
        void release(const KEY &key, const OBJ *obj)
        {
            auto search = m_map.find(key);

            if (search != m_map.end())
            {
                if (search->second.expired())
                {
                    m_map.erase(key);
                }
                else
                {
                    std::shared_ptr<OBJ> sp = m_map[key].lock();

                    if (sp.get() == obj)
                    {
                        m_map.erase(key);
                    }
                }
            }
        }

        /*
         * Release the object from the DB store, if it's the one we have stored
         */
        void add(const KEY &key, std::shared_ptr<OBJ> sp)
        {
            m_map[key] = sp;
        }

    private:
        std::map<KEY, std::weak_ptr<OBJ>> m_map;
    };
};

#endif
