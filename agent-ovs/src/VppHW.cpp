/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <memory>

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

void HW::CmdQ::enqueue(Cmd *f)
{
}

rc_t HW::CmdQ::write()
{
    return (rc_t::OK);
}

HW::CmdQ* HW::m_fifo;

/**
 * Initalse the connection to VPP
 */
void HW::init(HW::CmdQ *f)
{
    m_fifo = f;
    /**
     * Read all the existing state from VPP
     */
}
/**
 * Initalse the connection to VPP
 */
void HW::init()
{
    /**
     * Read all the existing state from VPP
     */
}

void HW::enqueue(Cmd *f)
{
    m_fifo->enqueue(f);
}

rc_t HW::write()
{
    return (m_fifo->write());
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
