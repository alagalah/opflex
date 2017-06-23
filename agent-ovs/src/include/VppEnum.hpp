/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ENUM_H__
#define __VPP_ENUM_H__

#include <string>

namespace VPP
{
    template <typename T>
    class Enum
    {
    public:
        const std::string &to_string() const
        {
            return (m_desc);
        }

        bool operator==(const Enum &e) const
        {
            return (e.m_value == m_value);
        }

        Enum & operator=(const Enum &e)
        {
            m_value = e.m_value;
            m_desc = e.m_desc;
        }

        bool operator!=(const Enum &e) const
        {
            return (e.m_value != m_value);
        }

        constexpr operator int() const
        {
            return (m_value);
        }

        constexpr int value() const
        {
            return (m_value);
        }

    protected:
        constexpr Enum(int value,
                       const std::string desc):
            m_value(value),
            m_desc(desc)
        {
        }
        virtual ~Enum()
        {
        }

    private:
        std::string m_desc;
        int m_value;
    };
};

#endif
