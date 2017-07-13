/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_VXLAN_TUNNEL_H__
#define __VPP_VXLAN_TUNNEL_H__

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppRoute.hpp"
#include "VppRouteDomain.hpp"
#include "VppInterface.hpp"

extern "C"
{
    #include "vxlan.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of a VXLAN Tunnel in VPP
     */
    class VxlanTunnel: public Interface
    {
    public:
        /**
         * Combaintion of attributes that are a unique key
         * for a VXLAN tunnel
         */
        struct endpoint_t
        {
            /**
             * Default constructor
             */
            endpoint_t();
            /**
             * Constructor taking endpoint values
             */
            endpoint_t(const boost::asio::ip::address &src,
                       const boost::asio::ip::address &dst,
                       uint32_t vni);

            /**
             * less-than operator for map storage
             */
            bool operator<(const endpoint_t &o) const;

            /**
             * Comparison operator
             */
            bool operator==(const endpoint_t &o) const;

            /**
             * Debug print function
             */
            std::string to_string() const;

            /**
             * The src IP address of the endpoint
             */
            boost::asio::ip::address src;

            /**
             * The destination IP address of the endpoint
             */
            boost::asio::ip::address dst;

            /**
             * The VNI of the endpoint
             */
            uint32_t vni;
        };

        /**
         * Construct a new object matching the desried state
         */
        VxlanTunnel(const boost::asio::ip::address &src,
                    const boost::asio::ip::address &dst,
                    uint32_t vni);

        /**
         * Construct a new object matching the desried state with a handle
         * read from VPP
         */
        VxlanTunnel(const handle_t &hdl,
                    const boost::asio::ip::address &src,
                    const boost::asio::ip::address &dst,
                    uint32_t vni);

        /*
         * Destructor
         */
        ~VxlanTunnel();

        /**
         * Copy constructor
         */
        VxlanTunnel(const VxlanTunnel& o);

        /**
         * Return the matching 'singular instance'
         */
        std::shared_ptr<VxlanTunnel> singular() const;

        /**
         * Debug rpint function
         */
        virtual std::string to_string() const;

        /**
         * Return VPP's handle to this object
         */
        const handle_t & handle() const;

        /**
         * Dump all L3Configs into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * replay the object to create it in hardware
         */
        void replay_i(void);

        /**
         * A Command class that creates an VXLAN tunnel
         */
        class CreateCmd: public Interface::CreateCmd
        {
        public:
            /**
             * Create command constructor taking HW item to update and the
             * endpoint values
             */
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name,
                      const endpoint_t &ep);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const CreateCmd&i) const;
        private:
            /**
             * Enpoint values of the tunnel to be created
             */
            const endpoint_t m_ep;
        };

        /**
         * A functor class that creates an VXLAN tunnel
         */
        class DeleteCmd: public Interface::DeleteCmd
        {
        public:
            /**
             * delete command constructor taking HW item to update and the
             * endpoint values
             */
            DeleteCmd(HW::Item<handle_t> &item,
                      const endpoint_t &ep);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const DeleteCmd&i) const;
        private:
            /**
             * Enpoint values of the tunnel to be deleted
             */            
            const endpoint_t m_ep;
        };

        /**
         * A cmd class that Dumps all the Vpp Interfaces
         */
        class DumpCmd: public VPP::DumpCmd<vapi_payload_vxlan_tunnel_details>
        {
        public:
            /**
             * Default Constructor
             */
            DumpCmd();

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const DumpCmd&i) const;
        };

    private:
        /**
         * populate state from VPP
         */
        static void populate(const KEY &key);

        /**
         * Populate VPP from current state, on VPP restart
         */
        static void replay(void);

        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const VxlanTunnel &obj);

        /**
         * Return the matching 'instance' of the sub-interface
         *  over-ride from the base class
         */
        std::shared_ptr<Interface> singular_i() const;

        /**
         * Find the VXLAN tunnel in the OM
         */
        static std::shared_ptr<VxlanTunnel> find_or_add(const VxlanTunnel &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * Tunnel enpoint/key
         */
        endpoint_t m_tep;

        /**
         * A map of all VLAN tunnela against thier key
         */
        static SingularDB<endpoint_t, VxlanTunnel> m_db;

        /**
         * Construct a unique name for the tunnel
         */
        static std::string mk_name(const boost::asio::ip::address &src,
                                   const boost::asio::ip::address &dst,
                                   uint32_t vni);
    };

    /**
     * Ostream output for a tunnel endpoint
     */
    std::ostream & operator<<(std::ostream &os, const VxlanTunnel::endpoint_t &ep);
};
#endif
