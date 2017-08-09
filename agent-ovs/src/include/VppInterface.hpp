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

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppEventCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppEnum.hpp"
#include "VppRoute.hpp"
#include "VppRouteDomain.hpp"
#include "VppDumpCmd.hpp"
#include "VppInspect.hpp"

#include <vapi/vapi.hpp>
#include <vapi/vpe.api.vapi.hpp>
#include <vapi/interface.api.vapi.hpp>
#include <vapi/af_packet.api.vapi.hpp>
#include <vapi/tap.api.vapi.hpp>

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
             */
            const static type_t TAP;

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
         * Return the matching'singular' of the interface
         */
        std::shared_ptr<Interface> singular() const;

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
         * Return the L2 Address
         */
        const l2_address_t & l2_address() const;

        /**
         * Set the operational state of the interface, as reported by VPP
         */
        void set(const oper_state_t &state);


        /**
         * A base class for Interface Create commands
         */
        template <typename MSG>
        class CreateCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>,
                                       MSG>
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                      const std::string &name):
                RpcCmd<HW::Item<handle_t>, HW::Item<handle_t>, MSG>(item),
                m_name(name)
            {
            }

            /**
             * Destructor
             */
            virtual ~CreateCmd() = default;

             /**
              * Comparison operator - only used for UT
              */
            virtual bool operator==(const CreateCmd &o) const
            {
                return (m_name == o.m_name);
            }

            /**
             * Indicate the succeeded, when the HW Q is disabled.
              */
            void succeeded()
            {
                RpcCmd<HW::Item<handle_t>, HW::Item<handle_t>, MSG>::succeeded();
                Interface::add(m_name, this->item());
            }

            virtual vapi_error_e operator() (MSG &reply)
            {
                int sw_if_index = reply.get_response().get_payload().sw_if_index;
                int retval = reply.get_response().get_payload().retval;

                BOOST_LOG_SEV(logger(), levels::debug) << this->to_string() << " " << retval;

                rc_t rc = rc_t::from_vpp_retval(retval);
                handle_t handle = handle_t::INVALID;

                if (rc_t::OK == rc)
                {
                    handle = sw_if_index;
                }

                HW::Item<handle_t> res(handle, rc);

                this->fulfill(res);

                return (VAPI_OK);
            }

        protected:
             /**
              * The name of the interface to be created
              */
            const std::string &m_name;
        };

        /**
         * A command class to create Loopback interfaces in VPP
         */
        class LoopbackCreateCmd: public CreateCmd<vapi::Create_loopback>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            LoopbackCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);
            ~LoopbackCreateCmd() = default;

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;
        };

        /**
         * A command class to create af_packet interfaces in VPP
         */
        class AFPacketCreateCmd: public CreateCmd<vapi::Af_packet_create>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            AFPacketCreateCmd(HW::Item<handle_t> &item,
                              const std::string &name);
            ~AFPacketCreateCmd() = default;
            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);
            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;
       };

        /**
         * A command class to create TAP interfaces in VPP
         */
        class TapCreateCmd: public CreateCmd<vapi::Tap_connect>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             * and the name of the interface to create
             */
            TapCreateCmd(HW::Item<handle_t> &item,
                         const std::string &name);
            ~TapCreateCmd() = default;

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(Connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;
        };

        /**
         * Base class for intterface Delete commands
         */
        template <typename MSG>
        class DeleteCmd: public RpcCmd<HW::Item<handle_t>,
                                       HW::Item<handle_t>,
                                       MSG>
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item,
                      const std::string &name):
                RpcCmd<HW::Item<handle_t>, HW::Item<handle_t>, MSG>(item),
                m_name(name)
            {
            }

            DeleteCmd(HW::Item<handle_t> &item):
                RpcCmd<HW::Item<handle_t>, HW::Item<handle_t>, MSG>(item),
                m_name()
            {
            }

            /**
             * Destructor
             */
            virtual ~DeleteCmd() = default;

             /**
              * Comparison operator - only used for UT
              */
            virtual bool operator==(const DeleteCmd &o) const
            {
                return (this->m_hw_item == o.m_hw_item);
            }

            /**
             * Indicate the succeeded, when the HW Q is disabled.
              */
            void succeeded()
            {
            }

        protected:
             /**
              * The name of the interface to be created
              */
            const std::string m_name;
        };

        /**
         * A command class to delete loopback interfaces in VPP
         */
        class LoopbackDeleteCmd: public DeleteCmd<vapi::Delete_loopback>
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
        };

        /**
         * A command class to delete af-packet interfaces in VPP
         */
        class AFPacketDeleteCmd: public DeleteCmd<vapi::Af_packet_delete>
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
        };

        /**
         * A command class to delete TAP interfaces in VPP
         */
        class TapDeleteCmd: public DeleteCmd<vapi::Tap_delete>
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
        };

        /**
         * A command class to delete TAP interfaces in VPP
         */
        class SetTag: public RpcCmd<HW::Item<handle_t>,
                                    rc_t,
                                    vapi::Sw_interface_tag_add_del>
       {
        public:
            /**
             * Constructor taking the HW::Item to update
             */
            SetTag(HW::Item<handle_t> &item,
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
            bool operator==(const SetTag&i) const;
        private:
            /**
             * The tag to add
             */
            const std::string m_name;
        };

        /**
         * A cmd class that changes the admin state
         */
        class StateChangeCmd: public RpcCmd<HW::Item<admin_state_t>,
                                            rc_t,
                                            vapi::Sw_interface_set_flags>
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
        class SetTableCmd: public RpcCmd<HW::Item<Route::table_id_t>,
                                         rc_t,
                                         vapi::Sw_interface_set_table>
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
        class EventsCmd: public RpcCmd<HW::Item<bool>,
                                       rc_t,
                                       vapi::Want_interface_events>,
                         public EventCmd<vapi::Msg<vapi_msg_sw_interface_set_flags>>
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
            void notify();
        private:
            /**
             * The listeners to notify when data/events arrive
             */
            EventListener & m_listener;
        };

        /**
         * A cmd class that Dumps all the Vpp Interfaces
         */
        class DumpCmd: public VPP::DumpCmd<vapi::Sw_interface_dump>
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

        /**
         * The the singular instance of the Interface in the Object-Model
         */
        static std::shared_ptr<Interface> find(const Interface &temp);

        /**
         * The the singular instance of the Interface in the Object-Model by handle
         */
        static std::shared_ptr<Interface> find(const handle_t &h);

        /**
         * The the singular instance of the Interface in the Object-Model by name
         */
        static std::shared_ptr<Interface> find(const std::string &s);

        /**
         * Dump all interfaces into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * Factory method to construct a new Interface from the VPP record
         */
        static std::unique_ptr<Interface> new_interface(const vapi_payload_sw_interface_details &vd);

    protected:
        /**
         * Construct an interface object with a handle and a HW address
         */
        Interface(const handle_t &handle,
                  const l2_address_t &l2_address,
                  const std::string &name,
                  type_t type,
                  admin_state_t state);

        /**
         * The SW interface handle VPP has asigned to the interface
         */
        HW::Item<handle_t> m_hdl;

        /**
         * Return the matching 'singular' of the interface
         */
        virtual std::shared_ptr<Interface> singular_i() const;

        /**
         * release/remove an interface form the singular store
         */
        void release();

        /**
         * Virtual functions to construct an interface create commands.
         * Overridden in derived classes like the SubInterface
         */
        virtual std::queue<Cmd*> & mk_create_cmd(std::queue<Cmd*> &cmds);

        /**
         * Virtual functions to construct an interface delete commands.
         * Overridden in derived classes like the SubInterface
         */
        virtual std::queue<Cmd*> & mk_delete_cmd(std::queue<Cmd*> &cmds);

        /**
         * Sweep/reap the object if still stale
         */
        virtual void sweep(void);

        /**
         * A map of all interfaces key against the interface's name
         */
        static SingularDB<const std::string, Interface> m_db;

        /**
         * Add an interface to the DB keyed on handle
         */
        static void add(const std::string &name,
                        const HW::Item<handle_t> &item);

        /**
         * remove an interface from the DB keyed on handle
         */
        static void remove(const HW::Item<handle_t> &item);

    private:
        /**
         * Class definition for listeners to OM events
         */
        class EventHandler: public OM::Listener, public Inspect::CommandHandler
        {
        public:
            EventHandler();
            virtual ~EventHandler() = default;

            /**
             * Handle a populate event
             */
            void handle_populate(const KeyDB::key_t & key);

            /**
             * Handle a replay event
             */
            void handle_replay();

            /**
             * Show the object in the Singular DB
             */
            void show(std::ostream &os);

            /**
             * Get the sortable Id of the listener
             */
            dependency_t order() const;
        };

        static EventHandler m_evh;

        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const Interface &obj);

        /*
         * It's the VPP::OM class that calls singular()
         */
        friend class VPP::OM;

        /**
         * It's the VPP::SingularDB class that calls replay()
         */
        friend class VPP::SingularDB<const std::string, Interface>;

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
         * HW state of the L2 address
         */
        HW::Item<l2_address_t> m_l2_address;

        /**
         * Operational state of the interface
         */
        oper_state_t m_oper;

        /**
         * A map of all interfaces keyed against VPP's handle
         */
        static std::map<handle_t, std::weak_ptr<Interface>> m_hdl_db;

        /**
         * replay the object to create it in hardware
         */
        virtual void replay(void);

        /**
         * Create commands are firends so they can add interfaces to the
         * handle store.
         */
        template <typename MSG> friend class CreateCmd;

        /**
         * Create commands are firends so they can remove interfaces from the
         * handle store.
         */
        template <typename MSG> friend class DeleteCmd;
    };
};
#endif
