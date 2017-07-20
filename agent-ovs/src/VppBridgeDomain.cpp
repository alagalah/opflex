/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <iostream>

#include "VppInterface.hpp"
#include "VppL2Binding.hpp"
#include "VppBridgeDomain.hpp"
#include "VppCmd.hpp"

using namespace VPP;

/**
 * A DB of al the interfaces, key on the name
 */
SingularDB<uint32_t, BridgeDomain> BridgeDomain::m_db;

BridgeDomain::EventHandler BridgeDomain::m_evh;

/**
 * Construct a new object matching the desried state
 */
BridgeDomain::BridgeDomain(uint32_t id):
    m_id(id)
{
}

BridgeDomain::BridgeDomain(const BridgeDomain& o):
    m_id(o.m_id)
{
}

uint32_t BridgeDomain::id() const
{
    return (m_id.data());
}

void BridgeDomain::sweep()
{
    if (rc_t::OK == m_id.rc())
    {
        HW::enqueue(new DeleteCmd(m_id));
    }
    HW::write();
}

void BridgeDomain::replay()
{
   if (rc_t::OK == m_id.rc())
   {
       HW::enqueue(new CreateCmd(m_id));
   }
}

BridgeDomain::~BridgeDomain()
{
    sweep();

    // not in the DB anymore.
    m_db.release(m_id.data(), this);
}

std::string BridgeDomain::to_string() const
{
    std::ostringstream s;
    s << "bridge-domain:["
      << m_id.to_string()
      << "]";

    return (s.str());
}

void BridgeDomain::update(const BridgeDomain &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_id.rc())
    {
        HW::enqueue(new CreateCmd(m_id));
    }
}

std::shared_ptr<BridgeDomain> BridgeDomain::find_or_add(const BridgeDomain &temp)
{
    return (m_db.find_or_add(temp.m_id.data(), temp));
}

std::shared_ptr<BridgeDomain> BridgeDomain::singular() const
{
    return find_or_add(*this);
}

void BridgeDomain::dump(std::ostream &os)
{
    m_db.dump(os);
}

void BridgeDomain::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    /*
     * dump VPP Bridge domains
     */
    BridgeDomain::DumpCmd::details_type *record;
    std::shared_ptr<BridgeDomain::DumpCmd> cmd(new BridgeDomain::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    while (record = cmd->pop())
    {
        BridgeDomain bd(record->bd_id);

        LOG(ovsagent::DEBUG) << "dump: " << bd.to_string();

        /*
         * Write each of the discovered interfaces into the OM,
         * but disable the HW Command q whilst we do, so that no
         * commands are sent to VPP
         */
        VPP::OM::commit(key, bd);

        /**
         * For each interface in the BD construct an L2Binding
         */
        for (int ii = 0; ii < record->n_sw_ifs; ii++)
        {
            std::shared_ptr<Interface> itf =
                Interface::find(record->sw_if_details[ii].sw_if_index);
            L2Binding l2(*itf, bd);
            OM::commit(key, l2);
        }

        free(record);
    }
}

BridgeDomain::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"bd", "bridge"}, "Bridge Domains", this);
}

void BridgeDomain::EventHandler::handle_replay()
{
    m_db.replay();
}

dependency_t BridgeDomain::EventHandler::order() const
{
    return (dependency_t::FORWARDING_DOMAIN);
}

void BridgeDomain::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
