/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <memory>

#include "logging.h"

#include "VppHW.hpp"

#include <vapi/vpe.api.vapi.hpp>

using namespace VPP;

HW::CmdQ::CmdQ():
    m_enabled(true),
    m_connected(false),
    m_conn()
{
}

HW::CmdQ::~CmdQ()
{
    m_connected = false;

    if (m_rx_thread && m_rx_thread->joinable())
    {
        m_rx_thread->join();
    }
}

HW::CmdQ & HW::CmdQ::operator=(const HW::CmdQ &f)
{
}

/**
 * Run the connect/dispatch thread.
 */
void HW::CmdQ::rx_run()
{
    while (m_connected)
    {
        m_conn.ctx().dispatch();
    }
}

void HW::CmdQ::enqueue(Cmd *cmd)
{
    std::shared_ptr<Cmd> sp(cmd);

    m_queue.push_back(sp);
}

void HW::CmdQ::enqueue(std::shared_ptr<Cmd> cmd)
{
    m_queue.push_back(cmd);
}

void HW::CmdQ::enqueue(std::queue<Cmd*> &cmds)
{
    while (cmds.size())
    {
        std::shared_ptr<Cmd> sp(cmds.front());

        m_queue.push_back(sp);
        cmds.pop();
    }
}

void HW::CmdQ::connect()
{
    if (m_connected)
    {
        m_conn.disconnect();
    }

    m_connected = false;

    if (m_rx_thread && m_rx_thread->joinable())
    {
        m_rx_thread->join();
    }

    m_conn.connect();

    m_connected = true;
    m_rx_thread.reset(new std::thread(&HW::CmdQ::rx_run, this));
}

void HW::CmdQ::enable()
{
    m_enabled = true;
}

void HW::CmdQ::disable()
{
    m_enabled = false;
}

rc_t HW::CmdQ::write()
{
    rc_t rc = rc_t::OK;

    /*
     * The queue is enabled, Execute each command in the queue.
     * If one execution fails, abort the rest
     */
    auto it = m_queue.begin();

    while (it != m_queue.end())
    {
        std::shared_ptr<Cmd> cmd = *it;

        LOG(ovsagent::DEBUG) << *cmd;

        if (m_enabled)
        {
            /*
             * before we issue the command we must move it to the pending store
             * ince a async event can be recieved before the command completes
             */
            m_pending[cmd.get()] = cmd;

            rc = cmd->issue(m_conn);

            if (rc_t::INPROGRESS == rc)
            {
                /*
                 * this command completes asynchronously
                 * leave the command in the pending store
                 */
            }
            else
            {
                /*
                 * the command completed, remove from the pending store
                 */
                m_pending.erase(cmd.get());

                if (rc_t::OK == rc)
                {
                    /*
                     * move to the next
                     */
                }
                else
                {
                    /*
                     * barf out without issuing the rest
                     */
                    break;
                }
            }
        }
        else
        {
            /*
             * The HW is disabled, so set each command as succeeded
             */
            cmd->succeeded();
        }

        ++it;
    }

    /*
     * erase all objects in the queue
     */
    m_queue.erase(m_queue.begin(), m_queue.end());

    return (rc);
}

/*
 * The single Command Queue
 */
HW::CmdQ* HW::m_cmdQ;
HW::Item<bool> HW::m_poll_state;

/**
 * Initalse the connection to VPP
 */
void HW::init(HW::CmdQ *f)
{
    m_cmdQ = f;
}

/**
 * Initalse the connection to VPP
 */
void HW::init()
{
    m_cmdQ = new CmdQ();
}

void HW::enqueue(Cmd *cmd)
{
    m_cmdQ->enqueue(cmd);
}

void HW::enqueue(std::shared_ptr<Cmd> cmd)
{
    m_cmdQ->enqueue(cmd);
}

void HW::enqueue(std::queue<Cmd*> & cmds)
{
    m_cmdQ->enqueue(cmds);
}

void HW::connect()
{
    m_cmdQ->connect();
}

void HW::enable()
{
    m_cmdQ->enable();
}

void HW::disable()
{
    m_cmdQ->disable();
}

rc_t HW::write()
{
    return (m_cmdQ->write());
}

bool HW::poll()
{
    std::shared_ptr<VPP::Cmd> poll(new Poll(m_poll_state));

    VPP::HW::enqueue(poll);
    VPP::HW::write();

    return (m_poll_state);
}

template <> std::string HW::Item<bool>::to_string() const
{
    std::ostringstream os;

    os << "hw-item:["
       << "rc:" << item_rc.to_string()
       << " data:" << item_data
       << "]";
    return (os.str());
}

template <> std::string HW::Item<unsigned int>::to_string() const
{
    std::ostringstream os;

    os << "hw-item:["
       << "rc:" << item_rc.to_string()
       << " data:" << item_data
       << "]";
    return (os.str());
}

HW::Poll::Poll(HW::Item<bool> &item):
    RpcCmd(item)
{
}

rc_t HW::Poll::issue(Connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return (rc_t::OK);
}

std::string HW::Poll::to_string() const
{
    std::ostringstream s;

    s << "poll: " << m_hw_item.to_string();

    return (s.str());
}
