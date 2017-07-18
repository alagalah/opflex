/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <iostream>
#include <algorithm>

#include "VppDhcpConfig.hpp"

DEFINE_VAPI_MSG_IDS_DHCP_API_JSON;

using namespace VPP;

DhcpConfig::BindCmd::BindCmd(HW::Item<bool> &item,
                             const handle_t &itf,
                             const std::string &hostname,
                             const std::vector<uint8_t> &client_id):
    RpcCmd(item),
    m_itf(itf),
    m_hostname(hostname),
    m_client_id(client_id)
{
}

bool DhcpConfig::BindCmd::operator==(const BindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_hostname == other.m_hostname));
}

rc_t DhcpConfig::BindCmd::issue(Connection &con)
{
    vapi_msg_dhcp_client_config*req;

    req = vapi_alloc_dhcp_client_config(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.is_add = 1;
    req->payload.pid = getpid();
    req->payload.want_dhcp_event = 1;
    
    memcpy(req->payload.hostname,
           m_hostname.c_str(),
           std::min(sizeof(req->payload.hostname),
                           m_hostname.length()));

    std::copy_n(m_client_id.begin(),
                std::min(sizeof(req->payload.client_id),
                         m_client_id.size()),
                req->payload.client_id);

    VAPI_CALL(vapi_dhcp_client_config(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_dhcp_client_config_reply,
                                   BindCmd>,
                  this));

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string DhcpConfig::BindCmd::to_string() const
{
    std::ostringstream s;
    s << "Dhcp-config-bind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " hostname:" << m_hostname;

    return (s.str());
}

DhcpConfig::UnbindCmd::UnbindCmd(HW::Item<bool> &item,
                                 const handle_t &itf,
                                 const std::string &hostname):
    RpcCmd(item),
    m_itf(itf),
    m_hostname(hostname)
{
}

bool DhcpConfig::UnbindCmd::operator==(const UnbindCmd& other) const
{
    return ((m_itf == other.m_itf) &&
            (m_hostname == other.m_hostname));
}

rc_t DhcpConfig::UnbindCmd::issue(Connection &con)
{
    vapi_msg_dhcp_client_config*req;

    req = vapi_alloc_dhcp_client_config(con.ctx());
    req->payload.sw_if_index = m_itf.value();
    req->payload.is_add = 0;
    req->payload.pid = getpid();
    req->payload.want_dhcp_event = 0;
    
    memcpy(req->payload.hostname,
           m_hostname.c_str(),
           std::min(sizeof(req->payload.hostname),
                           m_hostname.length()));
    
    VAPI_CALL(vapi_dhcp_client_config(
                  con.ctx(),
                  req,
                  RpcCmd::callback<vapi_payload_dhcp_client_config_reply,
                                   BindCmd>,
                  this));

    wait();
    m_hw_item.set(rc_t::NOOP);

    return rc_t::OK;
}

std::string DhcpConfig::UnbindCmd::to_string() const
{
    std::ostringstream s;
    s << "Dhcp-config-unbind: " << m_hw_item.to_string()
      << " itf:" << m_itf.to_string()
      << " hostname:" << m_hostname;

    return (s.str());
}

DhcpConfig::EventsCmd::EventsCmd(EventListener &el):
    RpcCmd(el.status()),
    EventCmd(),
    m_listener(el)
{
}

bool DhcpConfig::EventsCmd::operator==(const EventsCmd& other) const
{
    return (true);
}

rc_t DhcpConfig::EventsCmd::issue(Connection &con)
{
    vapi_msg_want_interface_events *req;

    /*
     * Set the call back to handle DHCP complete envets.
     */
    vapi_set_vapi_msg_dhcp_compl_event_event_cb(con.ctx(),
                                                EventCmd::callback<EventsCmd>,
                                                this);

    /*
     * return in-progress so the command stays in the pending list.
     */
    return (rc_t::INPROGRESS);
}

void DhcpConfig::EventsCmd::retire()
{
}

void DhcpConfig::EventsCmd::notify(vapi_payload_dhcp_compl_event *data)
{
    m_listener.handle_dhcp_event(this);
}

std::string DhcpConfig::EventsCmd::to_string() const
{
    return ("dhcp-events");
}
