/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ACL_BINDING_H__
#define __VPP_ACL_BINDING_H__

#include <ostream>
#include <string>
#include <stdint.h>

#include "VppObject.hpp"
#include "VppOM.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppAclTypes.hpp"
#include "VppInterface.hpp"
#include "VppAclList.hpp"
#include "VppInspect.hpp"

namespace VPP
{
    namespace ACL
    {
        /**
         * A Binding between an ACL and an interface.
         * A representation of the application of the ACL to the interface.
         */
        template <typename LIST, typename DETAILS>
        class Binding: public Object
        {
        public:
            /**
             * The key for a binding is the direction and the interface
             */
            typedef std::pair<direction_t, Interface::key_type> key_t;

            /**
             * Construct a new object matching the desried state
             */
            Binding(const direction_t &direction,
                    const Interface &itf,
                    const LIST &acl):
                m_direction(direction),
                m_itf(itf.singular()),
                m_acl(acl.singular()),
                m_binding(0)
            {
                m_evh.order();
            }

            /**
             * Copy Constructor
             */
            Binding(const Binding& o):
                m_direction(o.m_direction),
                m_itf(o.m_itf),
                m_acl(o.m_acl),
                m_binding(0)
            {
            }

            /**
             * Destructor
             */
            ~Binding()
            {
                sweep();
                m_db.release(std::make_pair(m_direction, m_itf->key()), this);
            }

            /**
             * Return the 'singular instance' of the L2 config that matches this object
             */
            std::shared_ptr<Binding> singular() const
            {
                return find_or_add(*this);
            }

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const
            {
                std::ostringstream s;
                s << "acl-binding:["
                  << m_itf->to_string()
                  << " " << m_acl->to_string()
                  << " " << m_binding.to_string()
                  << "]";

                return (s.str());
            }

            /**
             * Dump all Bindings into the stream provided
             */
            static void dump(std::ostream &os)
            {
                m_db.dump(os);
            }

            /**
             * A command class that binds the ACL to the interface
             */
            class BindCmd: public RpcCmd<HW::Item<bool>, rc_t>
            {
            public:
                /**
                 * Constructor
                 */
                BindCmd(HW::Item<bool> &item,
                        const direction_t &direction,
                        const handle_t &itf,
                        const handle_t &acl):
                    RpcCmd(item),
                    m_direction(direction),
                    m_itf(itf),
                    m_acl(acl)
                {
                }

                /**
                 * Issue the command to VPP/HW
                 */
                rc_t issue(Connection &con);

                /**
                 * convert to string format for debug purposes
                 */
                std::string to_string() const
                {
                    std::ostringstream s;
                    s << "acl-bind:["
                      << m_direction.to_string()
                      << " itf:" << m_itf.to_string()
                      << " acl:" << m_acl.to_string()
                      << "]";

                    return (s.str());
                }

                /**
                 * Comparison operator - only used for UT
                 */
                bool operator==(const BindCmd&other) const
                {
                    return ((m_itf == other.m_itf) &&
                            (m_acl == m_acl));
                }

            private:
                /**
                 * The interface to bind to
                 */
                const handle_t m_itf;

                /**
                 * The ACL to bind
                 */
                const handle_t m_acl;

                /**
                 * The direction of the binding
                 */
                const direction_t m_direction;
            };

            /**
             * A command class that binds the ACL to the interface
             */
            class UnbindCmd: public RpcCmd<HW::Item<bool>, rc_t>
            {
            public:
                /**
                 * Constructor
                 */
                UnbindCmd(HW::Item<bool> &item,
                          const direction_t &direction,
                          const handle_t &itf,
                          const handle_t &acl):
                    RpcCmd(item),
                    m_direction(direction),
                    m_itf(itf),
                    m_acl(acl)
                {
                }

                /**
                 * Issue the command to VPP/HW
                 */
                rc_t issue(Connection &con);

                /**
                 * convert to string format for debug purposes
                 */
                std::string to_string() const
                {
                    std::ostringstream s;
                    s << "acl-unbind:["
                      << m_direction.to_string()
                      << " itf:" << m_itf.to_string()
                      << " acl:" << m_acl.to_string()
                      << "]";

                    return (s.str());
                }

                /**
                 * Comparison operator - only used for UT
                 */
                bool operator==(const UnbindCmd&other) const
                {
                    return ((m_itf == other.m_itf) &&
                            (m_acl == m_acl));
                }

            private:
                /**
                 * The interface to bind to
                 */
                const handle_t m_itf;

                /**
                 * The ACL to bind
                 */
                const handle_t m_acl;

                /**
                 * The direction of the binding
                 */
                const direction_t m_direction;
            };

            /**
             * A cmd class that Dumps all the ACLs
             */
            class DumpCmd: public VPP::DumpCmd<DETAILS>
            {
            public:
                /**
                 * Constructor
                 */
                DumpCmd() = default;
                DumpCmd(const DumpCmd &d) = default;

                /**
                 * Issue the command to VPP/HW
                 */
                rc_t issue(Connection &con);

                /**
                 * convert to string format for debug purposes
                 */
                std::string to_string() const
                {
                    return ("acl-list-dump");
                }

            private:
                /**
                 * HW reutrn code
                 */
                HW::Item<bool> item;
            };

        private:
            /**
             * Class definition for listeners to OM events
             */
            class EventHandler: public OM::Listener, public Inspect::CommandHandler
            {
            public:
                EventHandler()
                {
                    OM::register_listener(this);
                    Inspect::register_handler({"acl-binding"}, "ACL Bindings", this);
                }
                virtual ~EventHandler() = default;

                /**
                 * Handle a populate event
                 */
                void handle_populate(const KeyDB::key_t & key);

                /**
                 * Handle a replay event
                 */
                void handle_replay()
                {
                    m_db.replay();
                }

                /**
                 * Show the object in the Singular DB
                 */
                void show(std::ostream &os)
                {
                    m_db.dump(os);
                }

                /**
                 * Get the sortable Id of the listener
                 */
                dependency_t order() const
                {
                    return (dependency_t::BINDING);
                }
            };

            /**
             * EventHandler to register with OM
             */
            static EventHandler m_evh;

            /**
             * Enquue commonds to the VPP command Q for the update
             */
            void update(const Binding &obj)
            {
                if (!m_binding)
                {
                    HW::enqueue(new BindCmd(m_binding,
                                            m_direction,
                                            m_itf->handle(),
                                            m_acl->handle()));
                }
                HW::write();
            }

            /**
             * Find or Add the instance in the DB
             */
            static std::shared_ptr<Binding> find_or_add(const Binding &temp)
            {
                return (m_db.find_or_add(std::make_pair(temp.m_direction,
                                                        temp.m_itf->key()),
                                         temp));
            }

            /*
             * It's the VPP::OM class that calls singular()
             */
            friend class VPP::OM;

            /**
             * It's the VPP::SingularDB class that calls replay()
             */
            friend class VPP::SingularDB<key_t, Binding>;

            /**
             * Sweep/reap the object if still stale
             */
            void sweep(void)
            {
                if (m_binding)
                {
                    HW::enqueue(new UnbindCmd(m_binding,
                                              m_direction,
                                              m_itf->handle(),
                                              m_acl->handle()));
                }
                HW::write();
            }

            /**
             * Replay the objects state to HW
             */
            void replay(void)
            {
                if (m_binding)
                {
                    HW::enqueue(new BindCmd(m_binding,
                                            m_direction,
                                            m_itf->handle(),
                                            m_acl->handle()));
                }
            }

            /**
             * A reference counting pointer the interface that this L3 layer
             * represents. By holding the reference here, we can guarantee that
             * this object will outlive the interface
             */
            const std::shared_ptr<Interface> m_itf;
    
            /**
             * A reference counting pointer the ACL that this
             * interface is bound to. By holding the reference here, we can
             * guarantee that this object will outlive the BD.
             */
            const std::shared_ptr<LIST> m_acl;

            /**
             * The direction the of the packets on which to apply the ACL
             * input or output
             */
            const direction_t m_direction;

            /**
             * HW configuration for the binding. The bool representing the
             * do/don't bind.
             */
            HW::Item<bool> m_binding;

            /**
             * A map of all L2 interfaces key against the interface's handle_t
             */
            static SingularDB<key_t, Binding> m_db;
        };

        /**
         * Typedef the L3 Binding type
         */
        typedef Binding<L3List, vapi_payload_acl_interface_list_details> L3Binding;

        /**
         * Typedef the L2 Binding type
         */
        typedef Binding<L2List, vapi_payload_macip_acl_interface_get_reply> L2Binding;

        /**
         * Definition of the static Singular DB for ACL Bindings
         */
        template <typename LIST, typename DETAILS>
        SingularDB<typename ACL::Binding<LIST, DETAILS>::key_t,
                   ACL::Binding<LIST, DETAILS>> Binding<LIST, DETAILS>::m_db;
        
        template <typename LIST, typename DETAILS>
        typename ACL::Binding<LIST, DETAILS>::EventHandler Binding<LIST, DETAILS>::m_evh;
    };

    std::ostream &operator<<(std::ostream &os,
                             const std::pair<ACL::direction_t,
                                             Interface::key_type> &key);
};

#endif
