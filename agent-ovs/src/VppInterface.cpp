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
#include "VppRoute.hpp"
#include "VppCmd.hpp"
#include "VppL3Binding.hpp"

using namespace VPP;

/**
 * A DB of all the interfaces, key on the name
 */
SingularDB<const std::string, Interface> Interface::m_db;

/**
 * A DB of all the interfaces, key on VPP's handle
 */
std::map<handle_t, std::weak_ptr<Interface>> Interface::m_hdl_db;

Interface::EventHandler Interface::m_evh;

/**
 * Construct a new object matching the desried state
 */
Interface::Interface(const std::string &name,
                     Interface::type_t itf_type,
                     Interface::admin_state_t itf_state):
    m_name(name),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_table_id(Route::DEFAULT_TABLE),
    m_l2_address(l2_address_t::ZERO, rc_t::UNSET),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const handle_t &handle,
                     const l2_address_t &l2_address,
                     const std::string &name,
                     Interface::type_t type,
                     Interface::admin_state_t state):
    m_hdl(handle),
    m_name(name),
    m_state(state),
    m_type(type),
    m_table_id(Route::DEFAULT_TABLE),
    m_l2_address(l2_address),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const std::string &name,
                     Interface::type_t itf_type,
                     Interface::admin_state_t itf_state,
                     const RouteDomain &rd):
    m_name(name),
    m_state(itf_state),
    m_type(itf_type),
    m_hdl(handle_t::INVALID),
    m_rd(rd.singular()),
    m_table_id(m_rd->table_id()),
    m_l2_address(l2_address_t::ZERO, rc_t::UNSET),
    m_oper(oper_state_t::DOWN)
{
}

Interface::Interface(const Interface& o):
    m_name(o.m_name),
    m_state(o.m_state),
    m_type(o.m_type),
    m_table_id(o.m_table_id),
    m_rd(o.m_rd),
    m_hdl(o.m_hdl),
    m_l2_address(o.m_l2_address),
    m_oper(o.m_oper)
{
}

Interface::EventListener::EventListener():
    m_status(rc_t::NOOP)
{
}

HW::Item<bool> &Interface::EventListener::status()
{
    return (m_status);
}

/**
 * Return the interface type
 */
const Interface::type_t & Interface::type() const
{
    return (m_type);
}

const handle_t & Interface::handle() const
{
    return (m_hdl.data());
}

const l2_address_t & Interface::l2_address() const
{
    return (m_l2_address.data());
}

void Interface::sweep()
{
    if (m_table_id)
    {
        m_table_id.data() = Route::DEFAULT_TABLE;
        HW::enqueue(new SetTableCmd(m_table_id, m_hdl));
    }

    // If the interface is up, bring it down
    if (m_state &&
        Interface::admin_state_t::UP == m_state.data())
    {
        m_state.data() = Interface::admin_state_t::DOWN;
        HW::enqueue(new StateChangeCmd(m_state, m_hdl));
    }
    if (m_hdl)
    {
        std::queue<Cmd*> cmds;
        HW::enqueue(mk_delete_cmd(cmds));
    }
    HW::write();
}

void Interface::replay()
{
   if (m_hdl)
   {
       std::queue<Cmd*> cmds;
       HW::enqueue(mk_create_cmd(cmds));
   }

   if (m_state &&
       Interface::admin_state_t::UP == m_state.data())
   {
       HW::enqueue(new StateChangeCmd(m_state, m_hdl));
   }

   if (m_table_id)
   {
       HW::enqueue(new SetTableCmd(m_table_id, m_hdl));
   }
}

Interface::~Interface()
{
    sweep();
    release();
}

void Interface::release()
{
    // not in the DB anymore.
    m_db.release(m_name, this);
}

std::string Interface::to_string() const
{
    std::ostringstream s;
    s << "interface:[" << m_name
      << " type:" << m_type.to_string()
      << " hdl:" << m_hdl.to_string()
      << " l2-address:" << m_l2_address.to_string();

    if (m_rd)
    {
        s << " rd:" << m_rd->to_string();
    }

    s << " admin-state:" << m_state.to_string()
      << " oper-state:" << m_oper.to_string()
      << "]";

    return (s.str());
}

const std::string &Interface::name() const
{
    return (m_name);
}

const Interface::key_type &Interface::key() const
{
    return (name());
}

std::queue<Cmd*> & Interface::mk_create_cmd(std::queue<Cmd*> &q)
{
    if (type_t::LOOPBACK == m_type)
    {
        q.push(new LoopbackCreateCmd(m_hdl, m_name));
    }
    else if (type_t::BVI == m_type)
    {
        q.push(new LoopbackCreateCmd(m_hdl, m_name));
        q.push(new SetTag(m_hdl, m_name));
    }
    else if (type_t::AFPACKET == m_type)
    {
        q.push(new AFPacketCreateCmd(m_hdl, m_name));
    }
    else if (type_t::TAP == m_type)
    {
        q.push(new TapCreateCmd(m_hdl, m_name));
    }

    return (q);
}

std::queue<Cmd*> & Interface::mk_delete_cmd(std::queue<Cmd*> &q)
{
    if ((type_t::LOOPBACK == m_type) ||
        (type_t::BVI == m_type))
    {
        q.push(new LoopbackDeleteCmd(m_hdl));
    }
    else if (type_t::AFPACKET == m_type)
    {
        q.push(new AFPacketDeleteCmd(m_hdl, m_name));
    }
    else if (type_t::TAP == m_type)
    {
        q.push(new TapDeleteCmd(m_hdl));
    }

    return (q);
}


void Interface::update(const Interface &desired)
{
    /*
     * the desired state is always that the interface should be created
     */
    if (rc_t::OK != m_hdl.rc())
    {
        std::queue<Cmd*> cmds;
        HW::enqueue(mk_create_cmd(cmds));
    }

    /*
     * change the interface state to that which is deisred
     */
    if (m_state.update(desired.m_state))
    {
        HW::enqueue(new StateChangeCmd(m_state, m_hdl));
    }

    /*
     * If the interface is mapped into a route domain, set VPP's
     * table ID
     */
    if (!m_table_id && m_rd)
    {
        HW::enqueue(new SetTableCmd(m_table_id, m_hdl));
    }        
}

void Interface::set(const oper_state_t &state)
{
    m_oper = state;
}

std::shared_ptr<Interface> Interface::singular_i() const
{
    return (m_db.find_or_add(name(), *this));
}

std::shared_ptr<Interface> Interface::singular() const
{
    return singular_i();
}

std::shared_ptr<Interface> Interface::find(const std::string &name)
{
    return (m_db.find(name));
}

std::shared_ptr<Interface> Interface::find(const handle_t &handle)
{
    return (m_hdl_db[handle].lock());
}

void Interface::add(const std::string &name,
                    const HW::Item<handle_t> &item)
{
    std::shared_ptr<Interface> sp = find(name);

    if (sp && item)
    {
        m_hdl_db[item.data()] = sp;
    }
}

void Interface::remove(const HW::Item<handle_t> &item)
{
    m_hdl_db.erase(item.data());
}

void Interface::dump(std::ostream &os)
{
    m_db.dump(os);
}

void Interface::EventHandler::handle_populate(const KeyDB::key_t &key)
{
    /*
     * dump VPP current states
     */
    std::shared_ptr<Interface::DumpCmd> cmd(new Interface::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    for (auto & itf_record : *cmd) //while (data = cmd->pop())
    {
        std::unique_ptr<Interface> itf = Interface::new_interface(itf_record.get_payload());

        /*
         * 'local' interface is internal to VPP and has no meaning to external clients.
         */
        if (itf && Interface::type_t::LOCAL != itf->type())
        {
            LOG(ovsagent::DEBUG) << "dump: " << itf->to_string();
            /*
             * Write each of the discovered interfaces into the OM,
             * but disable the HW Command q whilst we do, so that no
             * commands are sent to VPP
             */
            VPP::OM::commit(key, *itf);

            /**
             * Get the address configured on the interface
             */
            std::shared_ptr<L3Binding::DumpV4Cmd> dcmd =
                std::make_shared<L3Binding::DumpV4Cmd>(L3Binding::DumpV4Cmd(itf->handle()));

            HW::enqueue(dcmd);
            HW::write();

            for (auto &l3_record : *dcmd)
            {
                auto &payload = l3_record.get_payload();
                const Route::prefix_t pfx(payload.is_ipv6,
                                          payload.ip,
                                          payload.prefix_length);

                LOG(ovsagent::DEBUG) << "dump: " << pfx.to_string();

                L3Binding l3(*itf, pfx);
                OM::commit(key, l3);
            }
        }
    }
}

Interface::EventHandler::EventHandler()
{
    OM::register_listener(this);
    Inspect::register_handler({"interface", "intf"}, "Interfaces", this);
}

void Interface::EventHandler::handle_replay()
{
    m_db.replay();
}

dependency_t Interface::EventHandler::order() const
{
    return (dependency_t::INTERFACE);
}

void Interface::EventHandler::show(std::ostream &os)
{
    m_db.dump(os);
}
