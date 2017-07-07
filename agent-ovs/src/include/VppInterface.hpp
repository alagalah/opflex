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
            /**
             * Unkown type
             */
            const static type_t UNKNOWN;
            /**
             * A brideged Virtual interface (aka SVI or IRB)
             */
            const static type_t BVI;
            /**
             * VXLAN interface
             */
            const static type_t VXLAN;
            /**
             * Ethernet interface type
             */
            const static type_t ETHERNET;
            /**
             * AF-Packet interface type
             */
            const static type_t AFPACKET;
            /**
             * loopback interface type
             */
            const static type_t LOOPBACK;
            /**
             * Local interface type (specific to VPP)
             */
            const static type_t LOCAL;
            /**
             * TAP interface type
             */const static type_t TAP;

            /**
             * Convert VPP's name of the interface to a type
             */
            static type_t from_string(const std::string &str);
        private:
            /**
             * Private constructor taking the value and the string name
             */
            type_t(int v, const std::string &s);
        };

        /**
         * The admin state of the interface
         */
        struct admin_state_t: Enum<admin_state_t>
        {
            /**
             * Admin DOWN state
             */
            const static admin_state_t DOWN;
            /**
             * Admin UP state
             */
            const static admin_state_t UP;

            /**
             * Convert VPP's numerical value to Enum type
             */
            static admin_state_t from_int(uint8_t val);
        private:
            /**
             * Private constructor taking the value and the string name
             */
            admin_state_t(int v, const std::string &s);
        };

        /**
         * The oper state of the interface
         */
        struct oper_state_t: Enum<oper_state_t>
        {            
            /**
             * Operational DOWN state
             */
            const static oper_state_t DOWN;
            /**
             * Operational UP state
             */
            const static oper_state_t UP;

            /**
             * Convert VPP's numerical value to Enum type
             */
            static oper_state_t from_int(uint8_t val);
        private:
            /**
             * Private constructor taking the value and the string name
             */
            oper_state_t(int v, const std::string &s);
        };

        /**
         * Construct a new object matching the desried state
         */
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state);
        /**
         * Construct an interface object from a representation read from VPP
         */
        Interface(const vapi_payload_sw_interface_details &vd);
        /**
         * Construct a new object matching the desried state mapped
         * to a specific RouteDomain
         */
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state,
                  const RouteDomain &rd);
        /**
         * Destructor
         */
        virtual ~Interface();

        /**
         * Copy Constructor
         */
        Interface(const Interface& o);

        /**
         * Return the matching'instance' of the interface
         */
        std::shared_ptr<Interface> instance() const;

        /**
         * convert to string format for debug purposes
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
         * A base command class that creates an interface
         */
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            CreateCmd(HW::Item<handle_t> &item,
                         const std::string &name);
            /**
             * Destructor
             */virtual ~CreateCmd();

            /**
             * Comparison operator - only used for UT
             */
            virtual bool operator==(const CreateCmd&i) const;

            /**
             * Indiate the command is complete and the interface can be
             * added to the DB based on VPP's handle
             */
            void complete();
        protected:
            /**
             * The name of the interface to be created
             */
            const std::string &m_name;
        };

        /**
         * A command class to create Loopback interfaces in VPP
         */
        class LoopbackCreateCmd: public CreateCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            LoopbackCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);

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
            bool operator==(const LoopbackCreateCmd&i) const;
        };

        /**
         * A command class to create af_packet interfaces in VPP
         */
        class AFPacketCreateCmd: public CreateCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            AFPacketCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);

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
            bool operator==(const AFPacketCreateCmd&i) const;
        };

        /**
         * A command class to create TAP interfaces in VPP
         */
        class TapCreateCmd: public CreateCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            TapCreateCmd(HW::Item<handle_t> &item,
                         const std::string &name);

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
            bool operator==(const TapCreateCmd&i) const;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
        {
        public:
            /**
             * Constructor
             */
            DeleteCmd(HW::Item<handle_t> &item);
            /**
             * Constructor
             */
            DeleteCmd(HW::Item<handle_t> &item,
                      const std::string &name);
            /**
             * Destructor
             */
            virtual ~DeleteCmd();

            /**
             * Comparison operator - only used for UT
             */
            virtual bool operator==(const DeleteCmd&i) const;

            /**
             * Called when the command completes to remove the
             * interface from the per-handle store
             */
            void complete();
        protected:
            /**
             * the name of the interface to delete
             */
            const std::string m_name;
        };

        /**
         * A command class to delete loopback interfaces in VPP
         */
        class LoopbackDeleteCmd: public DeleteCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             */
            LoopbackDeleteCmd(HW::Item<handle_t> &item);

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
            bool operator==(const LoopbackDeleteCmd&i) const;
        };

        /**
         * A command class to delete af-packet interfaces in VPP
         */
        class AFPacketDeleteCmd: public DeleteCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to delete
             */
            AFPacketDeleteCmd(HW::Item<handle_t> &item,
                              const std::string &name);

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
            bool operator==(const AFPacketDeleteCmd&i) const;
        };

        /**
         * A command class to delete TAP interfaces in VPP
         */
        class TapDeleteCmd: public DeleteCmd
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             */
            TapDeleteCmd(HW::Item<handle_t> &item);

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
            bool operator==(const TapDeleteCmd&i) const;
        };

        /**
         * A cmd class that changes the admin state
         */
        class StateChangeCmd: public RpcCmd<HW::Item<admin_state_t>, rc_t>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name handle of the interface whose state is to change
             */
            StateChangeCmd(HW::Item<admin_state_t> &s,
                           const HW::Item<handle_t> &h);

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
            bool operator==(const StateChangeCmd&i) const;
        private:
            /**
             * the handle of the interface to update
             */
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * A command class that binds an interface to an L3 table
         */
        class SetTableCmd: public RpcCmd<HW::Item<Route::table_id_t>, rc_t>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name handle of the interface whose table is to change
             */
            SetTableCmd(HW::Item<Route::table_id_t> &item,
                         const HW::Item<handle_t> &h);

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
            bool operator==(const SetTableCmd&i) const;
        private:
            /**
             * the handle of the interface to update
             */
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * Forward declaration of the Evnet command
         */
        class EventsCmd;

        /**
         * A class that listens to Interface Events
         */
        class EventListener
        {
        public:
            /**
             * Default Constructor
             */
            EventListener();

            /**
             * Virtual function called on the listener when the command has data
             * ready to process
             */
            virtual void handle_interface_event(EventsCmd *cmd) = 0;

            /**
             * Return the HW::item representing the status
             */
            HW::Item<bool> &status();

        protected:
            /**
             * The status of the subscription
             */
            HW::Item<bool> m_status;
        };

        /**
         * A command class represents our desire to recieve interface events
         */
        class EventsCmd: public RpcCmd<HW::Item<bool>, rc_t>,
                         public EventCmd<vapi_msg_sw_interface_set_flags>
        {
        public:
            /**
             * Constructor taking the listner to notify
             */
            EventsCmd(EventListener &el);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);

            /**
             * Retires the command - unsubscribe from the events.
             */
            void retire();

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const EventsCmd&i) const;

            /**
             * Called when it's time to poke the listeners
             */
            void notify(vapi_msg_sw_interface_set_flags *data);
        private:
            /**
             * The listeners to notify when data/events arrive
             */
            EventListener & m_listener;
        };

        /**
         * A cmd class that Dumps all the Vpp Interfaces
         */
        class DumpInterfaceCmd: public DumpCmd<vapi_payload_sw_interface_details>
        {
        public:
            /**
             * Default Constructor
             */
            DumpInterfaceCmd();

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
            bool operator==(const DumpInterfaceCmd&i) const;
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

        /**
         * The the instance of the Interface in the Object-Model by handle
         */
        static std::shared_ptr<Interface> find(const handle_t &h);

        /**
         * The the instance of the Interface in the Object-Model by name
         */
        static std::shared_ptr<Interface> find(const std::string &s);

    protected:
        /**
         * The SW interface handle VPP has asigned to the interface
         */
        HW::Item<handle_t> m_hdl;

        /**
         * Return the matching 'instance' of the interface
         */
        virtual std::shared_ptr<Interface> instance_i() const;

        /**
         * release/remove an interface form the instance store
         */
        void release();

        /**
         * Virtual functions to construct an interface create commands.
         * Overridden in derived classes like the SubInterface
         */
        virtual Cmd* mk_create_cmd();

        /**
         * Virtual functions to construct an interface delete commands.
         * Overridden in derived classes like the SubInterface
         */
        virtual Cmd* mk_delete_cmd();

        /**
         * Sweep/reap the object if still stale
         */
        virtual void sweep(void);

        /**
         * A map of all interfaces key against the interface's name
         */
        static InstDB<const std::string, Interface> m_db;

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const Interface &obj);

        /*
         * It's the VPP::OM class that calls instance()
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
        const type_t m_type;

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
         * A map of all interfaces keyed against VPP's handle
         */
        static std::map<handle_t, std::weak_ptr<Interface>> m_hdl_db;

        /**
         * Add an interface to the DB keyed on handle
         */
        static void add(const handle_t &hdl, std::shared_ptr<Interface> sp);

        /**
         * remove an interface from the DB keyed on handle
         */
        static void remove(const handle_t &hdl);

        /**
         * Create commands are firends so they can add interfaces to the
         * handle store.
         */
        friend class CreateCmd;

        /**
         * Create commands are firends so they can remove interfaces from the
         * handle store.
         */
        friend class DeleteCmd;
    };
};
#endif
