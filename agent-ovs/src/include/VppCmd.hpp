/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_FUNCTUOR_H__
#define __VPP_FUNCTUOR_H__

#include <stdint.h>
#include <string>

#include "Vpp.hpp"

namespace VPP
{
    /**
     * A representation of a method call to VPP
     */
    class Cmd
    {
    public:
        Cmd(uint8_t priority)
        {
        }
        virtual ~Cmd()
        {
        }

        virtual rc_t exec() = 0;

        virtual std::string to_string() = 0;
    private:
        uint8_t m_prio;
    };
    
    template <typename HWITEM>
    class CmdT: public Cmd
    {
    public:
        CmdT(HWITEM &item):
            Cmd(0),
            m_hw_item(item)
        {
        }

        virtual ~CmdT()
        {
        }

        HWITEM & item()
        {
            return m_hw_item;
        }
        const HWITEM & item() const
        {
            return m_hw_item;
        }

    protected:
        HWITEM &m_hw_item;
    };
};


#endif
