/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_INTERFACE_H__
#define __VPP_INTERFACE_H__

#include <string>
#include <stdint.h>

#include "logging.h"

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppEventCmd.hpp"
#include "VppInstDB.hpp"
#include "VppEnum.hpp"
#include "VppRoute.hpp"
#include "VppRouteDomain.hpp"
#include "VppDumpCmd.hpp"

extern "C"
{
#include "vapi.h"
#include "interface.api.vapi.h"
#include "af_packet.api.vapi.h"
#include "tap.api.vapi.h"
}

namespace VPP
{
    /**
     * A representation of an interface in VPP
     */
    class Interface: public Object
    {
    public:
        /**
         * The key for interface's key
         */
        typedef std::string key_type;

        /**
         * An interface type
         */
        struct type_t: Enum<type_t>
        {
            const static type_t UNKNOWN;
            const static type_t BVI;
            const static type_t VXLAN;
            const static type_t ETHERNET;
            const static type_t AFPACKET;
            const static type_t LOOPBACK;
            const static type_t LOCAL;
            const static type_t TAP;

            static type_t from_string(const std::string &str);
        private:
            type_t(int v, const std::string &s);
        };

        /**
         * The admin state of the interface
         */
        struct admin_state_t: Enum<admin_state_t>
        {            
            const static admin_state_t DOWN;
            const static admin_state_t UP;

            static admin_state_t from_int(uint8_t val);
        private:
            admin_state_t(int v, const std::string &s);
        };

        /**
         * The oper state of the interface
         */
        struct oper_state_t: Enum<oper_state_t>
        {            
            const static oper_state_t DOWN;
            const static oper_state_t UP;

            static oper_state_t from_int(uint8_t val);
        private:
            oper_state_t(int v, const std::string &s);
        };

        /**
         * Construct a new object matching the desried state
         */
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state);
        Interface(const vapi_payload_sw_interface_details &vd);
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state,
                  const RouteDomain &rd);
        virtual ~Interface();
        Interface(const Interface& o);

        /**
         * Debug rpint function
         */
        virtual std::string to_string(void) const;

        /**
         * Return VPP's handle to this object
         */
        const handle_t & handle() const;

        /**
         * Return the interface type
         */
        const type_t & type() const;

        /**
         * Return the interface type
         */
        const std::string & name() const;

        /**
         * Return the interface type
         */
        const key_type & key() const;

        /**
         * Set the operational state of the interface, as reported by VPP
         */
        void set(const oper_state_t &state);

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name);

            virtual bool operator==(const CreateCmd&i) const;

            void complete();
        protected:
            const std::string &m_name;
        };

        class LoopbackCreateCmd: public CreateCmd
        {
        public:
            LoopbackCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const LoopbackCreateCmd&i) const;
        };

        class AFPacketCreateCmd: public CreateCmd
        {
        public:
            AFPacketCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const AFPacketCreateCmd&i) const;
        };

        class TapCreateCmd: public CreateCmd
        {
        public:
            TapCreateCmd(HW::Item<handle_t> &item,
                         const std::string &name);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const TapCreateCmd&i) const;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item);

            virtual bool operator==(const DeleteCmd&i) const;
            void complete();
        };

        class LoopbackDeleteCmd: public DeleteCmd
        {
        public:
            LoopbackDeleteCmd(HW::Item<handle_t> &item);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const LoopbackDeleteCmd&i) const;
        };

        class AFPacketDeleteCmd: public DeleteCmd
        {
        public:
            AFPacketDeleteCmd(HW::Item<handle_t> &item);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const AFPacketDeleteCmd&i) const;
        };

        class TapDeleteCmd: public DeleteCmd
        {
        public:
            TapDeleteCmd(HW::Item<handle_t> &item);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const TapDeleteCmd&i) const;
        };

        /**
         * A cmd class that changes the admin state
         */
        class StateChangeCmd: public RpcCmd<HW::Item<admin_state_t>, rc_t>
        {
        public:
            StateChangeCmd(HW::Item<admin_state_t> &s,
                            const HW::Item<handle_t> &h);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const StateChangeCmd&i) const;
        private:
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * A functor class that binds an interface to an L3 table
         */
        class SetTableCmd: public RpcCmd<HW::Item<Route::table_id_t>, rc_t>
        {
        public:
            SetTableCmd(HW::Item<Route::table_id_t> &item,
                         const HW::Item<handle_t> &h);

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const SetTableCmd&i) const;
        private:
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * A class that listens to Interface Events
         */
        class EventsCmd;
        class EventListener
        {
        public:
            EventListener();
            virtual void handle_interface_event(EventsCmd *cmd) = 0;

            HW::Item<bool> & status();
        protected:
            HW::Item<bool> m_status;
        };

        /**
         * A functor class represents our desire to recieve interface events
         */
        class EventsCmd: public RpcCmd<HW::Item<bool>, rc_t>,
                         public EventCmd<vapi_msg_sw_interface_set_flags>
        {
        public:
            EventsCmd(EventListener &el);

            rc_t issue(Connection &con);
            void retire();
            std::string to_string() const;

            bool operator==(const EventsCmd&i) const;

            void notify(vapi_msg_sw_interface_set_flags *data);
        private:
            EventListener & m_listener;
        };

        /**
         * A cmd class that Dumps all the Vpp Interfaces
         */
        class DumpInterfaceCmd: public DumpCmd<vapi_payload_sw_interface_details>
        {
        public:
            DumpInterfaceCmd();

            rc_t issue(Connection &con);
            std::string to_string() const;

            bool operator==(const DumpInterfaceCmd&i) const;
        private:
            HW::Item<bool> item;
        };

        /**
         * A generic callback function for handling Interface crete complete
         * callbacks from VPP
         */
        template <typename REPLY, typename CMD_TYPE>
        static vapi_error_e create_callback(vapi_ctx_t ctx,
                                            void *callback_ctx,
                                            vapi_error_e rv,
                                            bool is_last,
                                            REPLY *reply)
        {
            CMD_TYPE *cmd = static_cast<CMD_TYPE*>(callback_ctx);

            LOG(ovsagent::INFO) << cmd->to_string() << " " << reply->retval;

            HW::Item<handle_t> res(reply->sw_if_index,
                                   rc_t::from_vpp_retval(reply->retval));

            cmd->fulfill(res);

            return (VAPI_OK);
        }

        /**
         * The the instance of the Interface in the Object-Model
         */
        static std::shared_ptr<Interface> find(const Interface &temp);
        static std::shared_ptr<Interface> find(const handle_t &h);
        static std::shared_ptr<Interface> find(const std::string &s);

    protected:
        /**
         * The SW interface handle VPP has asigned to the interface
         */
        HW::Item<handle_t> m_hdl;

        static std::shared_ptr<Interface> find_or_add(const Interface &temp);
        static void insert(const Interface &temp, std::shared_ptr<Interface> sp);
        void release();

        /**
         * Virtual functions to construct an interface create/delete commands.
         * Overridden in derived classes like the SubInterface
         */
        virtual Cmd* mk_create_cmd();
        virtual Cmd* mk_delete_cmd();

        /**
         * Sweep/reap the object if still stale
         */
        virtual void sweep(void);

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const Interface &obj);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;

        /**
         * The interfaces name
         */
        const std::string m_name;
    
        /**
         * The interface type. clearly this cannot be changed
         * once the interface has been created.
         */
        type_t m_type;

        /**
         * The state of the interface
         */
        HW::Item<admin_state_t> m_state;

        /**
         * shared pointer to the RouteDoamin the interface is in.
         * NULL is not mapped  - i.e. in eht default table
         */
        const std::shared_ptr<RouteDomain> m_rd;

        /**
         * HW state of the VPP table mapping
         */
        HW::Item<Route::table_id_t> m_table_id;

        /**
         * Operational state of the interface
         */
        oper_state_t m_oper;

        /**
         * A map of all interfaces key against the interface's name
         */
        static InstDB<const std::string, Interface> m_db;

        /**
         * A map of all interfaces keyed against VPP's handle
         */
        static std::map<handle_t, std::weak_ptr<Interface>> m_hdl_db;

        static void add(const handle_t &hdl, std::shared_ptr<Interface> sp);
        static void remove(const handle_t &hdl);

        friend class CreateCmd;
        friend class DeleteCmd;
    };
};
#endif
