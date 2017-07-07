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
    /**
     * Atemplate base class for all Enum types.
     * This Enum type exists to associate an enum value with a string for
     * display/debug purposes.
     * Concrete Enum types use the CRTP. Derived classes thus inherit this
     * base's function, but are not polymorphic.
     */
    template <typename T>
    class Enum
    {
    public:
        /**
         * convert to string format for debug purposes
         */
        const std::string &to_string() const
        {
            return (m_desc);
        }

        /**
         * Comparison operator
         */
        bool operator==(const Enum &e) const
        {
            return (e.m_value == m_value);
        }

        /**
         * Assignemet
         */
        Enum & operator=(const Enum &e)
        {
            m_value = e.m_value;
            m_desc = e.m_desc;
        }

        /**
         * Comparison operator
         */
        bool operator!=(const Enum &e) const
        {
            return (e.m_value != m_value);
        }

        /**
         * integer conversion operator
         */
        constexpr operator int() const
        {
            return (m_value);
        }

        /**
         * Return the value of the enum - same as integer conversion
         */
        constexpr int value() const
        {
            return (m_value);
        }

    protected:
        /**
         * Constructor of an Enum - takes value and string description
         */
        constexpr Enum(int value,
                       const std::string desc):
            m_value(value),
            m_desc(desc)
        {
        }

        /**
         * Constructor
         */
        virtual ~Enum()
        {
        }

    private:
        /**
         * String description
         */
        std::string m_desc;

        /**
         * Integer value of the enum
         */
        int m_value;
    };
};

#endif
