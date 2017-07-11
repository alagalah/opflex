/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <algorithm>

#include "VppOM.hpp"
#include "VppInterface.hpp"
#include "VppVxlanTunnel.hpp"
#include "VppBridgeDomain.hpp"

using namespace VPP;

KeyDB *OM::m_db;

/**
 * Initalse the connection to VPP
 */
void OM::init()
{
    m_db = new KeyDB();
}

void OM::mark(const KEY &key)
{
    /*
     * Find if the object already stored on behalf of this key.
     * and mark them stale
     */
    VPP::ObjectRefList& objs = m_db->find(key);

    auto mark_obj = [](const ObjectRef &oref)
        {
            oref.mark();
        };

    std::for_each(objs.begin(), objs.end(), mark_obj);
}

void OM::sweep(const KEY &key)
{
    /*
     * Find if the object already stored on behalf of this key.
     * and mark them stale
     */
    VPP::ObjectRefList& objs = m_db->find(key);

    auto is_stale = [](const ObjectRef &oref)
        {
            return (oref.stale());
        };
    objs.erase(std::remove_if(objs.begin(), objs.end(), is_stale), objs.end());

    HW::write();
}

void OM::remove(const KEY &key)
{
    /*
     * Simply reset the list for this key. This will desctruct the
     * object list and shared_ptrs therein. When the last shared_ptr
     * goes the objects desctructor is called and the object is
     * removed from OM
     */
    m_db->flush(key);

    HW::write();
}

void OM::replay()
{
}

void OM::dump(const KEY & key, std::ostream &os)
{
    m_db->dump(key, os);
}

void OM::dump(std::ostream &os)
{
    m_db->dump(os);
}

void OM::populate(const KEY &key)
{
    /*
     * The ordering here is important. build from the bottom up.
     */
    Interface::populate(key);
    VxlanTunnel::populate(key);
    BridgeDomain::populate(key);

    /*
     * once we have it all, mark it stale.
     */
    mark(key);
}
