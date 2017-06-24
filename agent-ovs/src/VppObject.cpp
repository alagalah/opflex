/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppObject.hpp"
#include "VppObjectDB.hpp"

using namespace VPP;

/**
 * Construct a new object matching the desried state
 */
Object::Object()
{
    ObjectDB::add(this);
}

Object::~Object()
{
    ObjectDB::remove(this);
}

ObjectRef::ObjectRef(std::shared_ptr<Object> obj):
    m_obj(obj),
    m_state(OBJECT_STATE_NONE)
{
}

bool ObjectRef::operator<(const ObjectRef &other) const
{
    return (m_obj.get() < other.m_obj.get());
}

std::shared_ptr<Object> ObjectRef::obj() const
{
    return (m_obj);
}

void ObjectRef::mark()  const
{
    m_state = OBJECT_STATE_STALE;
}

void ObjectRef::clear() const
{
    m_state = OBJECT_STATE_NONE;
}

bool ObjectRef::stale() const
{
    return (m_state == OBJECT_STATE_STALE);
}

std::ostream& VPP::operator<<(std::ostream &os, const Object& o)
{
    os << o.to_string();

    return (os);
}
