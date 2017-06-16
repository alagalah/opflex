/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppObjectDB.hpp"

using namespace VPP;

/**
 * A map of all the objects
 */
std::map<const VPP::Object*, VPP::Object*> VPP::ObjectDB::m_objs;


/* VPP::ObjectDB::VPP::ObjectDBIter VPP::ObjectDB::begin() */
/* { */
/*     return (m_objs.begin()); */
/* } */
ObjectDB::iterator ObjectDB::begin()
{
    return (m_objs.begin());
}
ObjectDB::iterator ObjectDB::end()
{
    return (m_objs.end());
}

void ObjectDB::add(VPP::Object *obj)
{
    m_objs[obj] = obj;
}

void ObjectDB::remove(VPP::Object *obj)
{
    m_objs.erase(obj);
}
