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

/**
 * Convenince wrapper macro for error handling in VAPI sends
 */
#define VAPI_CALL(_stmt)                        \
{                                               \
    vapi_error_e _rv;                           \
    do                                          \
    {                                           \
        _rv = (_stmt);                          \
    } while (VAPI_OK != _rv);                   \
}

namespace VPP
{
   /**
    * Error codes that VPP will return during a HW write
    */
    struct rc_t: public Enum<rc_t>
    {
        /**
         * Constructor
         */
        rc_t(int v, const std::string s);

        /**
         * Destructor
         */
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
         * HW write is in progress. Also used for the 'want' events
         * that never complete
         */
        const static rc_t INPROGRESS;

        /**
         * HW write reported invalid input
         */
        const static rc_t INVALID;

        /**
         * HW write timedout - VPP did not respond within a timely manner
         */
        const static rc_t TIMEOUT;

        /**
         * Get the rc_t from the VPP API value
         */
        static const rc_t &from_vpp_retval(int32_t rv);
    };

    /**
     * A type declaration of an interface handle in VPP
     */
    struct handle_t
    {
        /**
         * Constructor
         */
        handle_t(int value);

        /**
         * Constructor
         */
        handle_t();

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Comparison operator
         */
        bool operator==(const handle_t &other) const;

        /**
         * less than operator
         */
        bool operator<(const handle_t &other) const;

        /**
         * A value of an interface handle_t that means the itf does not exist
         */
        const static handle_t INVALID;

        /**
         * get the value of the handle
         */
        uint32_t value() const;

    private:
        /**
         * VPP's handle value
         */
        uint32_t m_value;
    };

    /**
     * ostream print of a handle_t
     */
    std::ostream & operator<<(std::ostream &os, const handle_t &h);
};

#endif
