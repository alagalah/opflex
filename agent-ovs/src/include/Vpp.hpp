/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_H__
#define __VPP_H__

#include <boost/asio/ip/address.hpp>

#include "VppEnum.hpp"

namespace VPP
{
   /**
    * Error codes that VPP will return during a HW write
    */
    struct rc_t: public Enum<rc_t>
    {
        rc_t(int v, const std::string s);
        ~rc_t();

        /**
         * The value un-set
         */
        const static rc_t UNSET;

        /**
         * The HW write/update action was/has not been attempted
         */
        const static rc_t NOOP;

        /**
         * The HW write was successfull
         */
        const static rc_t OK;

        /**
         * HW write reported invalid input
         */
        const static rc_t INVALID;
    };

    /**
     * A type declaration of an interface handle in VPP
     */
    struct handle_t
    {
        handle_t(int value);
        std::string to_string() const;
        bool operator==(const handle_t &other) const;

        bool operator<(const handle_t &other) const;
        /**
         * A value of an interface handle_t that means the itf does not exist
         */
        const static handle_t INVALID;

        uint32_t value() const;

    private:
        uint32_t m_value;
    };

    std::ostream & operator<<(std::ostream &os, const handle_t &h);
};

#endif
