/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppKeyDB.hpp"

using namespace VPP;

ObjectRefList& KeyDB::find(const KeyDB::key_t &k)
{
    return (m_objs[k]);
}

void KeyDB::flush(const KeyDB::key_t &k)
{
    m_objs.erase(m_objs.find(k));
}
