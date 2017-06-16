/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ROUTE_H__
#define __VPP_ROUTE_H__

#include <boost/asio/ip/address.hpp>


namespace VPP
{
   /**
    * Error codes that VPP will return during a HW write
    */
    class Route
    {
    public:
        /**
         * A prefix defintion. Address + length
         */
        class prefix_t
        {
        public:
            prefix_t(const boost::asio::ip::address &addr,
                     uint8_t len);
            ~prefix_t();
            prefix_t(const std::string &s,
                     uint8_t len);
            prefix_t(const prefix_t&);

            const boost::asio::ip::address &address() const;

            prefix_t &operator=(const prefix_t&);
            bool operator<(const prefix_t &o) const;
            bool operator==(const prefix_t &o) const;
            bool operator!=(const prefix_t &o) const;

            std::string to_string() const;

            /**
             * The all Zeros prefix
             */
            const static prefix_t ZERO;

        private:
            boost::asio::ip::address m_addr;
            uint8_t m_len;
        };
    };
};

#endif
