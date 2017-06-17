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

using namespace VPP;

/**
 * Initalse the connection to VPP
 */
HW::CmdQ::CmdQ():
    m_conn(new ovsagent::VppConnection())
{
    /**
     * Read all the existing state from VPP
     */
}

HW::CmdQ::~CmdQ()
{
}

HW::CmdQ & HW::CmdQ::operator=(const HW::CmdQ &f)
{
}

void HW::CmdQ::enqueue(Cmd *cmd)
{
    std::shared_ptr<Cmd> sp(cmd);

    m_queue.push_back(sp);
}

rc_t HW::CmdQ::write()
{
    rc_t rc = rc_t::OK;

    /*
     * Execute each command in the queue. If one execution fails, abort the rest
     */
    auto it = m_queue.begin();

    while (it != m_queue.end())
    {
        LOG(ovsagent::INFO) << **it;

        rc = (*it)->exec();

        if (rc_t::OK == rc)
        {
            ++it;
        }
        else
        {
            break;
        }
    }

    /*
     * erase all objects in the equeue
     */
    m_queue.erase(m_queue.begin(), m_queue.end());

    return (rc);
}

HW::CmdQ* HW::m_cmdQ;

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

void HW::enqueue(Cmd *f)
{
    m_cmdQ->enqueue(f);
}

rc_t HW::write()
{
    return (m_cmdQ->write());
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
