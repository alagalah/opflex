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


using namespace VPP;

Interface::CreateCmd::CreateCmd(HW::Item<handle_t> &item,
                                const std::string &name):
    RpcCmd(item),
    m_name(name)
{
}

Interface::CreateCmd::~CreateCmd()
{
}

bool Interface::CreateCmd::operator==(const CreateCmd& other) const
{
    return (m_name == other.m_name);
}

void Interface::CreateCmd::complete()
{
    std::shared_ptr<Interface> sp = find(m_name);

    if (sp)
    {
        add(m_hw_item.data(), sp);
    }
}

void Interface::CreateCmd::succeeded()
{
    m_hw_item.set(rc_t::OK);

    complete();
}

Interface::LoopbackCreateCmd::LoopbackCreateCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::LoopbackCreateCmd::issue(Connection &con)
{
    vapi_msg_create_loopback *req;

    req = vapi_alloc_create_loopback(con.ctx());

    VAPI_CALL(vapi_create_loopback(con.ctx(), req,
                                   Interface::create_callback<
                                       vapi_payload_create_loopback_reply,
                                       LoopbackCreateCmd>,
                                   this));

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        complete();
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

bool Interface::LoopbackCreateCmd::operator==(const LoopbackCreateCmd& other) const
{
    return (CreateCmd::operator==(other));
}

Interface::AFPacketCreateCmd::AFPacketCreateCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::AFPacketCreateCmd::issue(Connection &con)
{
    vapi_msg_af_packet_create *req;

    req = vapi_alloc_af_packet_create(con.ctx());

    memset(req->payload.host_if_name, 0,
           sizeof(req->payload.host_if_name));
    memcpy(req->payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(req->payload.host_if_name)));

    VAPI_CALL(vapi_af_packet_create(con.ctx(), req,
                                    Interface::create_callback<
                                        vapi_payload_af_packet_create_reply,
                                        AFPacketCreateCmd>,
                                    this));

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        complete();
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

bool Interface::AFPacketCreateCmd::operator==(const AFPacketCreateCmd& other) const
{
    return (CreateCmd::operator==(other));
}

Interface::TapCreateCmd::TapCreateCmd(HW::Item<handle_t> &item,
                                      const std::string &name):
    CreateCmd(item, name)
{
}

rc_t Interface::TapCreateCmd::issue(Connection &con)
{
    vapi_msg_tap_connect *req;

    req = vapi_alloc_tap_connect(con.ctx());
    memset(req->payload.tap_name, 0,
                    sizeof(req->payload.tap_name));
    memcpy(req->payload.tap_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(req->payload.tap_name)));

    VAPI_CALL(vapi_tap_connect(con.ctx(), req,
                               Interface::create_callback<
                                   vapi_payload_tap_connect_reply,
                                   TapCreateCmd>,
                               this));

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        complete();
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

bool Interface::TapCreateCmd::operator==(const TapCreateCmd& other) const
{
    return (CreateCmd::operator==(other));
}

Interface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item):
    RpcCmd(item)
{
}

Interface::DeleteCmd::~DeleteCmd()
{
}

void Interface::DeleteCmd::complete()
{
    remove(m_hw_item.data());
}

Interface::DeleteCmd::DeleteCmd(HW::Item<handle_t> &item,
                                const std::string &name):
    RpcCmd(item),
    m_name(name)
{
}

bool Interface::DeleteCmd::operator==(const DeleteCmd& other) const
{
    return (m_hw_item == other.m_hw_item);
}

Interface::LoopbackDeleteCmd::LoopbackDeleteCmd(HW::Item<handle_t> &item):
    DeleteCmd(item)
{
}

rc_t Interface::LoopbackDeleteCmd::issue(Connection &con)
{
    vapi_msg_delete_loopback *req;

    req = vapi_alloc_delete_loopback(con.ctx());
    req->payload.sw_if_index = m_hw_item.data().value();

    VAPI_CALL(vapi_delete_loopback(
                  con.ctx(), req,
                  RpcCmd::callback<vapi_payload_delete_loopback_reply,
                                   LoopbackDeleteCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    complete();
    return rc_t::OK;
}
std::string Interface::LoopbackDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "loopback-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

bool Interface::LoopbackDeleteCmd::operator==(const LoopbackDeleteCmd& other) const
{
    return (DeleteCmd::operator==(other));
}

Interface::AFPacketDeleteCmd::AFPacketDeleteCmd(HW::Item<handle_t> &item,
                                                const std::string &name):
    DeleteCmd(item, name)
{
}

rc_t Interface::AFPacketDeleteCmd::issue(Connection &con)
{
    vapi_msg_af_packet_delete *req;

    req = vapi_alloc_af_packet_delete(con.ctx());
    memset(req->payload.host_if_name, 0,
           sizeof(req->payload.host_if_name));
    memcpy(req->payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(req->payload.host_if_name)));

    VAPI_CALL(vapi_af_packet_delete(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_af_packet_delete_reply,
                                   AFPacketDeleteCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    complete();
    return rc_t::OK;
}
std::string Interface::AFPacketDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "af_packet-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

bool Interface::AFPacketDeleteCmd::operator==(const AFPacketDeleteCmd& other) const
{
    return (DeleteCmd::operator==(other));
}

Interface::TapDeleteCmd::TapDeleteCmd(HW::Item<handle_t> &item):
    DeleteCmd(item)
{
}

rc_t Interface::TapDeleteCmd::issue(Connection &con)
{
    // finally... call VPP

    complete();
    return rc_t::OK;
}
std::string Interface::TapDeleteCmd::to_string() const
{
    std::ostringstream s;
    s << "tap-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

bool Interface::TapDeleteCmd::operator==(const TapDeleteCmd& other) const
{
    return (DeleteCmd::operator==(other));
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
    vapi_msg_sw_interface_set_flags *req;

    req = vapi_alloc_sw_interface_set_flags(con.ctx());
    req->payload.sw_if_index = m_hdl.data().value();
    req->payload.admin_up_down = m_hw_item.data().value();

    VAPI_CALL(vapi_sw_interface_set_flags(
                  con.ctx(), req,
                  RpcCmd::callback<vapi_payload_sw_interface_set_flags_reply,
                                   StateChangeCmd>,
                  this));

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
    vapi_msg_sw_interface_set_table *req;

    req = vapi_alloc_sw_interface_set_table(con.ctx());
    req->payload.sw_if_index = m_hdl.data().value();
    req->payload.is_ipv6 = 0;
    req->payload.vrf_id = m_hw_item.data();

    VAPI_CALL(vapi_sw_interface_set_table(con.ctx(),
                                          req,
                                          RpcCmd::callback<
                                              vapi_payload_sw_interface_set_table_reply,
                                          SetTableCmd>,
                                          this));

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
    vapi_msg_want_interface_events *req;

    /*
     * First set the clal back to handle the interface events
     */
    vapi_set_event_cb(con.ctx(),
                      vapi_msg_id_sw_interface_set_flags,
                      EventCmd::callback<EventsCmd>,
                      this);

    /*
     * then send the request to enable them
     */
    req = vapi_alloc_want_interface_events(con.ctx());

    req->payload.enable_disable = 1;
    req->payload.pid = getpid();

    VAPI_CALL(vapi_want_interface_events(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_want_interface_events_reply,
                                   EventsCmd>,
                  this));

    wait();

    return (rc_t::INPROGRESS);
}

void Interface::EventsCmd::retire()
{
}

void Interface::EventsCmd::notify(vapi_msg_sw_interface_set_flags *data)
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
    vapi_msg_sw_interface_dump *req;

    req = vapi_alloc_sw_interface_dump(con.ctx());
    req->payload.name_filter_valid = 0;

    VAPI_CALL(vapi_sw_interface_dump(con.ctx(), req,
                                     DumpCmd::callback<DumpCmd>,
                                     this));

    wait();

    return rc_t::OK;
}

std::string Interface::DumpCmd::to_string() const
{
    return ("Vpp-Interfaces-Dump");
}

Interface::SetTag::SetTag(HW::Item<handle_t> &item,
                          const std::string &name):
    RpcCmd(item),
    m_name(name)
{
}

rc_t Interface::SetTag::issue(Connection &con)
{
    vapi_msg_sw_interface_tag_add_del *req;

    req = vapi_alloc_sw_interface_tag_add_del(con.ctx());
    req->payload.is_add = 1;
    req->payload.sw_if_index = m_hw_item.data().value();
    memcpy(req->payload.tag, m_name.c_str(), m_name.length());

    VAPI_CALL(vapi_sw_interface_tag_add_del(
                  con.ctx(), req,
                  RpcCmd::callback<vapi_payload_sw_interface_tag_add_del_reply,
                  SetTag>,
                  this));

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