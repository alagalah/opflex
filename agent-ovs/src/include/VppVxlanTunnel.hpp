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
#include "VppInstDB.hpp"
#include "VppRoute.hpp"
#include "VppRouteDomain.hpp"
#include "VppInterface.hpp"

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
            endpoint_t();
            endpoint_t(const boost::asio::ip::address &src,
                        const boost::asio::ip::address &dst,
                        uint32_t vni);

            bool operator<(const endpoint_t &o) const;
            std::string to_string() const;

            boost::asio::ip::address src;
            boost::asio::ip::address dst;
            uint32_t vni;
        };

        /**
         * Construct a new object matching the desried state
         */
        VxlanTunnel(const boost::asio::ip::address &src,
                    const boost::asio::ip::address &dst,
                    uint32_t vni);
        ~VxlanTunnel();
        VxlanTunnel(const VxlanTunnel& o);

        /**
         * Debug rpint function
         */
        virtual std::string to_string() const;

        /**
         * Return VPP's handle to this object
         */
        const handle_t & handle() const;

        /**
         * A functor class that creates an VXLAN tunnel
         */
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      const boost::asio::ip::address &src,
                      const boost::asio::ip::address &dst,
                      uint32_t vni);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const CreateCmd&i) const;
        private:
            const boost::asio::ip::address m_src;
            const boost::asio::ip::address m_dst;
            uint32_t m_vni;
        };

        /**
         * A functor class that creates an VXLAN tunnel
         */
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item,
                      const boost::asio::ip::address &src,
                      const boost::asio::ip::address &dst,
                      uint32_t vni);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const DeleteCmd&i) const;
        private:
            const boost::asio::ip::address m_src;
            const boost::asio::ip::address m_dst;
            uint32_t m_vni;
        };

        /**
         * The the instance of the VxlanTunnel in the Object-Model
         */
        static std::shared_ptr<VxlanTunnel> find(const VxlanTunnel &temp);

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const VxlanTunnel &obj);

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
        static InstDB<endpoint_t, VxlanTunnel> m_db;

        /**
         * Construct a unique name for the tunnel
         */
        static std::string mk_name(const boost::asio::ip::address &src,
                                   const boost::asio::ip::address &dst,
                                   uint32_t vni);
    };
};
#endif
