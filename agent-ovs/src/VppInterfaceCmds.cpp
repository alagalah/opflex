/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "VppInterface.hpp"
#include "VppCmd.hpp"

DEFINE_VAPI_MSG_IDS_VPE_API_JSON;
DEFINE_VAPI_MSG_IDS_INTERFACE_API_JSON;
DEFINE_VAPI_MSG_IDS_AF_PACKET_API_JSON;
DEFINE_VAPI_MSG_IDS_TAP_API_JSON;

using namespace VPP;

Interface::LoopbackCreateCmd::LoopbackCreateCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::LoopbackCreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        Interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}
std::string Interface::LoopbackCreateCmd::to_string() const
{
    std::ostringstream s;
    s << "loopback-itf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

Interface::AFPacketCreateCmd::AFPacketCreateCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::AFPacketCreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();

    payload.use_random_hw_addr = 1;
    memset(payload.host_if_name, 0,
           sizeof(payload.host_if_name));
    memcpy(payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.host_if_name)));

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        Interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}
std::string Interface::AFPacketCreateCmd::to_string() const
{
    std::ostringstream s;
    s << "af-packet-itf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

Interface::TapCreateCmd::TapCreateCmd(HW::Item<handle_t> &item,
                                      const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::TapCreateCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();

    memset(payload.tap_name, 0,
           sizeof(payload.tap_name));
    memcpy(payload.tap_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.tap_name)));

    payload.use_random_mac = 1;

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        Interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}

std::string Interface::TapCreateCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-intf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

Interface::LoopbackDeleteCmd::LoopbackDeleteCmd(HW::Item<handle_t> &item):
    DeleteCmd(item)
{
}

rc_t Interface::LoopbackDeleteCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hw_item.data().value();

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    Interface::remove(m_hw_item);
    return rc_t::OK;
}

std::string Interface::LoopbackDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "loopback-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

Interface::AFPacketDeleteCmd::AFPacketDeleteCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    DeleteCmd(item, name)
{
}

rc_t Interface::AFPacketDeleteCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    memset(payload.host_if_name, 0,
           sizeof(payload.host_if_name));
    memcpy(payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.host_if_name)));

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    Interface::remove(m_hw_item);
    return rc_t::OK;
}
std::string Interface::AFPacketDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "af_packet-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

Interface::TapDeleteCmd::TapDeleteCmd(HW::Item<handle_t> &item):
    DeleteCmd(item)
{
}

rc_t Interface::TapDeleteCmd::issue(Connection &con)
{
    // finally... call VPP

    Interface::remove(m_hw_item);
    return rc_t::OK;
}
std::string Interface::TapDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

Interface::StateChangeCmd::StateChangeCmd(HW::Item<Interface::admin_state_t> &state,
                                            const HW::Item<handle_t> &hdl):
    RpcCmd(state),
    m_hdl(hdl)
{
}

bool Interface::StateChangeCmd::operator==(const StateChangeCmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::StateChangeCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hdl.data().value();
    payload.admin_up_down = m_hw_item.data().value();

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string Interface::StateChangeCmd::to_string() const
{
    std::ostringstream s;
    s << "itf-state-change: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}

Interface::SetTableCmd::SetTableCmd(HW::Item<Route::table_id_t> &table,
                                    const HW::Item<handle_t> &hdl):
    RpcCmd(table),
    m_hdl(hdl)
{
}

bool Interface::SetTableCmd::operator==(const SetTableCmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t Interface::SetTableCmd::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hdl.data().value();
    payload.is_ipv6 = 0;
    payload.vrf_id = m_hw_item.data();

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return (rc_t::OK);
}

std::string Interface::SetTableCmd::to_string() const
{
    std::ostringstream s;
    s << "itf-state-change: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}


Interface::EventsCmd::EventsCmd(EventListener &el):
    RpcCmd(el.status()),
    EventCmd(),
    m_listener(el)
{
}

bool Interface::EventsCmd::operator==(const EventsCmd& other) const
{
    return (true);
}

rc_t Interface::EventsCmd::issue(Connection &con)
{
    /*
     * First set the clal back to handle the interface events
     */
    m_reg.reset(new reg_t(con.ctx(), std::ref(*(static_cast<EventCmd*>(this)))));
    // m_reg->execute();

    /*
     * then send the request to enable them
     */
    msg_t req(con.ctx(), std::ref(*(static_cast<RpcCmd*>(this))));

    auto &payload = req.get_request().get_payload();
    payload.enable_disable = 1;
    payload.pid = getpid();

    VAPI_CALL(req.execute());

    wait();

    return (rc_t::INPROGRESS);
}

void Interface::EventsCmd::retire()
{
}

void Interface::EventsCmd::notify()
{
    m_listener.handle_interface_event(this);
}

std::string Interface::EventsCmd::to_string() const
{
    return ("itf-events");
}

Interface::DumpCmd::DumpCmd()
{
}

bool Interface::DumpCmd::operator==(const DumpCmd& other) const
{
    return (true);
}

rc_t Interface::DumpCmd::issue(Connection &con)
{
    m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

    auto &payload = m_dump->get_request().get_payload();
    payload.name_filter_valid = 0;

    VAPI_CALL(m_dump->execute());

    wait();

    return rc_t::OK;
}

std::string Interface::DumpCmd::to_string() const
{
    return ("itf-dump");
}

Interface::SetTag::SetTag(HW::Item<handle_t> &item,
                          const std::string &name):
    RpcCmd(item),
    m_name(name)
{
}

rc_t Interface::SetTag::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.is_add = 1;
    payload.sw_if_index = m_hw_item.data().value();
    memcpy(payload.tag, m_name.c_str(), m_name.length());

    VAPI_CALL(req.execute());

    wait();

    return rc_t::OK;
}
std::string Interface::SetTag::to_string() const
{
    std::ostringstream s;
    s << "itf-set-tag: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

bool Interface::SetTag::operator==(const SetTag& o) const
{
    return ((m_name == o.m_name) &&
            (m_hw_item.data() == o.m_hw_item.data()));
}
