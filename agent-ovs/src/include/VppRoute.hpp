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
    * Types belonging to Routing
    */
    namespace Route
    {
        /**
         * type def the table-id
         */
        typedef uint32_t table_id_t;

        /**
         * The table-id for the default table
         */
        const static table_id_t DEFAULT_TABLE = 0;

        /**
         * A prefix defintion. Address + length
         */
        class prefix_t
        {
        public:
            /**
             * Default Constructor - creates ::/0
             */
            prefix_t();
            /**
             * Constructor with address and length
             */
            prefix_t(const boost::asio::ip::address &addr,
                     uint8_t len);
            /**
             * Constructor with string and length
             */
            prefix_t(const std::string &s,
                     uint8_t len);
            /**
             * Copy Constructor
             */
            prefix_t(const prefix_t&);
            /**
             * Constructor with VPP API prefix representation
             */
            prefix_t(uint8_t is_ip6, uint8_t *addr, uint8_t len);
            /**
             * Destructor
             */
            ~prefix_t();

            /**
             * Get the address
             */
            const boost::asio::ip::address &address() const;

            /**
             * Assignement
             */
            prefix_t &operator=(const prefix_t&);

            /**
             * Less than operator
             */
            bool operator<(const prefix_t &o) const;

            /**
             * equals operator
             */
            bool operator==(const prefix_t &o) const;

            /**
             * not equal opartor
             */
            bool operator!=(const prefix_t &o) const;

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * The all Zeros prefix
             */
            const static prefix_t ZERO;

            /**
             * Convert the prefix into VPP API parameters
             */
            void to_vpp(uint8_t *is_ip6, uint8_t *addr, uint8_t *len) const;

        private:
            /**
             * The address
             */
            boost::asio::ip::address m_addr;

            /**
             * The prefix length
             */
            uint8_t m_len;
        };
    };

    /**
     * Convert a boost address into a VPP bytes string
     */
    void to_bytes(const boost::asio::ip::address &addr, uint8_t *is_ip6, uint8_t *array);
};

#endif
