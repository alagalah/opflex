/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_ACL_LIST_H__
#define __VPP_ACL_LIST_H__

#include <set>
#include <string>
#include <stdint.h>

#include "VppAclTypes.hpp"
#include "VppAclL3Rule.hpp"
#include "VppAclL2Rule.hpp"
#include "VppHW.hpp"
#include "VppRpcCmd.hpp"
#include "VppDumpCmd.hpp"
#include "VppSingularDB.hpp"
#include "VppOM.hpp"
#include "VppInspect.hpp"

extern "C"
{
    #include "acl.api.vapi.h"
}

namespace VPP
{
    namespace ACL
    {
        /**
         * An ACL list comprises a set of match actions rules to be applied to packets.
         * A list is bound to a given interface.
         */
        template <typename RULE, typename DETAILS>
        class List: public Object
        {
        public:
            /**
             * Dependency level 'ACL'
             */
            const static dependency_t dependency_value = dependency_t::ACL;

            /**
             * The KEY can be used to uniquely identify the ACL.
             * (other choices for keys, like the summation of the properties
             * of the rules, are rather too cumbersome to use
             */
            typedef std::string key_t;

            /**
             * The rule container type
             */
            typedef std::set<RULE> rules_t;

            /**
             * Construct a new object matching the desried state
             */
            List(const key_t &key):
                m_key(key)
            {
            }

            List(const handle_t &hdl,
                 const key_t &key):
                m_hdl(hdl),
                m_key(key)
            {
            }

            List(const key_t &key,
                 const rules_t &rules):
                m_key(key),
                m_rules(rules)
            {
                m_evh.order();
            }

            /**
             * Copy Constructor
             */
            List(const List& o):
                m_hdl(o.m_hdl),
                m_key(o.m_key),
                m_rules(o.m_rules)
            {
            }

            /**
             * Destructor
             */
            ~List()
             {
                 sweep();
                 m_db.release(m_key, this);
             }

            /**
             * Return the 'sigular instance' of the ACL that matches this object
             */
            std::shared_ptr<List> singular() const
            {
                return find_or_add(*this);
            }

            /**
             * Dump all ACLs into the stream provided
             */
            static void dump(std::ostream &os)
            {
                m_db.dump(os);
            }

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const
            {
                std::ostringstream s;
                s << "acl-list:[" << m_key
                  << " " << m_hdl.to_string()
                  << " rules:[";

                for (auto rule : m_rules)
                {
                    s << rule.to_string() << " ";
                }

                s << "]]";

                return (s.str());
            }

            /**
             * Insert priority sorted a rule into the list
             */
            void insert(const RULE &rule)
            {
                m_rules.insert(rule);
            }

            /**
             * Remove a rule from the list
             */
            void remove(const RULE &rule)
            {
                m_rules.erase(rule);
            }

            /**
             * Return the VPP assign handle
             */
            const handle_t &handle() const
            {
                return m_hdl.data();
            }

            /**
             * A command class that Create the List
             */
            class UpdateCmd: public RpcCmd<HW::Item<handle_t>,
                                           HW::Item<handle_t>>
            {
            public:
                /**
                 * Constructor
                 */
                UpdateCmd(HW::Item<handle_t> &item,
                          const key_t &key,
                          const rules_t &rules):
                    RpcCmd(item),
                    m_key(key),
                    m_rules(rules)
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
                    s << "ACL-list-update: " << m_hw_item.to_string();

                    return (s.str());
                }

                /**
                 * Comparison operator - only used for UT
                 */
                bool operator==(const UpdateCmd &other) const
                {
                    return ((m_key == other.m_key) &&
                            (m_rules == other.m_rules));
                }

                void complete()
                {
                    std::shared_ptr<List> sp = find(m_key);
                    if (sp && m_hw_item)
                    {
                        List::add(m_hw_item.data(), sp);
                    }
                }

                void succeeded()
                {
                    RpcCmd::succeeded();
                    complete();
                }

            private:
                /**
                 * The key.
                 */
                const key_t &m_key;

                /**
                 * The rules
                 */
                const rules_t &m_rules;
            };

            /**
             * A cmd class that Deletes an ACL
             */
            class DeleteCmd: public RpcCmd<HW::Item<handle_t>, rc_t>
            {
            public:
                /**
                 * Constructor
                 */
                DeleteCmd(HW::Item<handle_t> &item):
                    RpcCmd(item)
                {
                }

                /**
                 * Issue the command to VPP/HW
                 */
                rc_t issue(Connection &con)
                {
                    return (rc_t::INVALID);
                }

                /**
                 * convert to string format for debug purposes
                 */
                std::string to_string() const
                {
                    std::ostringstream s;
                    s << "ACL-list-delete: " << m_hw_item.to_string();

                    return (s.str());
                }

                /**
                 * Comparison operator - only used for UT
                 */
                bool operator==(const DeleteCmd &other) const
                {
                    return (m_hw_item.data() == other.m_hw_item.data());
                }
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
                rc_t issue(Connection &con)
                {
                    return rc_t::INVALID;
                }

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

                LOG(ovsagent::DEBUG) << cmd->to_string() << " " << reply->retval;

                HW::Item<handle_t> res(reply->acl_index,
                                       rc_t::from_vpp_retval(reply->retval));

                cmd->fulfill(res);

                return (VAPI_OK);
            }

            static std::shared_ptr<List> find(const handle_t &handle)
            {
                return (m_hdl_db[handle].lock());
            }

            static std::shared_ptr<List> find(const key_t &key)
            {
                return (m_db.find(key));
            }

            static void add(const handle_t &handle,
                            std::shared_ptr<List> sp)
            {
                m_hdl_db[handle] = sp;
            }

            static void remove(const handle_t &handle)
            {
                m_hdl_db.erase(handle);
            }

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
                    Inspect::register_handler({"acl"}, "ACL Lists", this);
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
                    return (dependency_t::ACL);
                }
            };

            /**
             * EventHandler to register with OM
             */
            static EventHandler m_evh;

            /**
             * Enquue commonds to the VPP command Q for the update
             */
            void update(const List &obj)
            {
                /*
                 * always update the instance with the latest rule set
                 */
                if (!m_hdl || obj.m_rules != m_rules)
                {
                    HW::enqueue(new UpdateCmd(m_hdl, m_key, m_rules));
                }
                /*
                 * We don't, can't, read the priority from VPP,
                 * so the is equals check above does not include the priorty.
                 * but we save it now.
                 */
                m_rules = obj.m_rules;
            }

            /**
             * HW assigned handle
             */
            HW::Item<handle_t> m_hdl;

            /**
             * Find or add the sigular instance in the DB
             */
            static std::shared_ptr<List> find_or_add(const List &temp)
            {
                return (m_db.find_or_add(temp.m_key, temp));
            }

            /*
             * It's the VPP::OM class that updates call update
             */
            friend class VPP::OM;

            /**
             * It's the VPP::SingularDB class that calls replay()
             */
            friend class VPP::SingularDB<key_t, List>;

            /**
             * Sweep/reap the object if still stale
             */
            void sweep(void)
            {
                if (m_hdl)
                {
                    HW::enqueue(new DeleteCmd(m_hdl));
                }
                HW::write();
            }

            /**
             * Replay the objects state to HW
             */
            void replay(void)
            {
                if (m_hdl)
                {
                    HW::enqueue(new UpdateCmd(m_hdl, m_key, m_rules));
                }
            }

            /**
             * A map of all ACL's against the client's key
             */
            static SingularDB<key_t, List> m_db;

            /**
             * A map of all ACLs keyed against VPP's handle
             */
            static std::map<const handle_t, std::weak_ptr<List>> m_hdl_db;

            /**
             * The Key is a user defined identifer for this ACL
             */
            const key_t m_key;

            /**
             * A sorted list of the rules
             */
            rules_t m_rules;
        };

        /**
         * Typedef the L3 ACL type
         */
        typedef List<L3Rule, vapi_payload_acl_details> L3List;

        /**
         * Typedef the L2 ACL type
         */
        typedef List<L2Rule, vapi_payload_macip_acl_details> L2List;

        /**
         * Definition of the static SingularDB for ACL Lists
         */
        template <typename RULE, typename DETAILS>
        SingularDB<typename ACL::List<RULE, DETAILS>::key_t,
                   ACL::List<RULE, DETAILS> > List<RULE, DETAILS>::m_db;

        /**
         * Definition of the static per-handle DB for ACL Lists
         */
        template <typename RULE, typename DETAILS>
        std::map<const handle_t,
                 std::weak_ptr<ACL::List<RULE, DETAILS> > > List<RULE, DETAILS>::m_hdl_db;

        template <typename RULE, typename DETAILS>
        typename ACL::List<RULE, DETAILS>::EventHandler List<RULE, DETAILS>::m_evh;

    };
};

#endif
