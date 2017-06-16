/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <algorithm>

#include "VppOM.hpp"
#include "VppObjectDB.hpp"

using namespace VPP;

KeyDB *OM::m_db;

/**
 * A special owner for all the state read from VPP HW at init time
 */
static const std::string hw_owner = "__VPP_HW__";

/**
 * Initalse the connection to VPP
 */
void OM::init()
{
    m_db = new KeyDB();

    /*
     * Read the existing state from the HW
     */

    /*
     * Mark all that state stale
     */
    mark(hw_owner);
}

/**
 * Initalse the connection to VPP
 */
void OM::converged()
{
    /*
     * Sweep all the stale state
     */
    sweep(hw_owner);
}

void OM::mark(KEY &key)
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

void OM::sweep(KEY &key)
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
