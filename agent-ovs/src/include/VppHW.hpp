/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_HW_H__
#define __VPP_HW_H__

#include <deque>
#include <map>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>

#include <vapi/vapi.hpp>
#include <vapi/vpe.api.vapi.hpp>

#include "VppRpcCmd.hpp"
#include "VppConnection.hpp"

namespace VPP
{
    class HW
    {
    public:
        /**
         * A HW::item is data that is either to be written to or read from VPP/HW.
         * The item is a pair of the data written/read and the result of that operation.
         */
        template <typename T>
        class Item
        {
        public:
            /**
             * Constructor
             */
            Item(const T &data):
                item_data(data),
                item_rc(rc_t::NOOP)
             {
             }
            /**
             * Constructor
             */
            Item():
                item_rc(rc_t::NOOP)
             {
             }

            /**
             * Constructor
             */
            Item(rc_t rc):
                item_rc(rc)
             {
             }

            /**
             * Constructor
             */
            Item(const T &data,
                 rc_t rc):
                item_data(data),
                item_rc(rc)
             {
             }

            /**
             * Comparison operator
             */
            bool operator==(const Item<T> &i) const
            {
                return (item_data == i.item_data);
            }

            /**
             * Copy assignment
             */
            Item & operator=(const Item &other)
            {
                item_data = other.item_data;
                item_rc = other.item_rc;
            }

            /**
             * Return the data read/written
             */
            T &data()
            {
                return (item_data);
            }

            /**
             * Const reference to the data
             */
            const T &data() const
            {
                return (item_data);
            }

            /**
             * Get the HW return code
             */
            rc_t rc() const
            {
                 return (item_rc);
            }

            /**
             * Set the HW return code - should only be called from the
             * family of Command objects
             */
            void set(const rc_t &rc)
            {
                item_rc = rc;
            }

            /**
             * Return true if the HW Item is configred in HW
             */
            operator bool() const
            {
                return (rc_t::OK == item_rc);
            }

            /**
             * update the item to the desired state.
             *  return true if a HW update is required
             */
            bool update(const Item &desired)
            {
                bool need_hw_update;

                /*
                 * A HW update is needed if the state is different
                 * and/or the state is not yet in HW
                 */
                need_hw_update = (item_data != desired.data() |
                                  (rc_t::OK != rc()));

                item_data = desired.data();

                return (need_hw_update);
            }

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const
            {
                std::ostringstream os;

                os << "hw-item:["
                   << "rc:" << item_rc.to_string()
                   << " data:" << item_data.to_string()
                   << "]";

                return (os.str());
            }

        private:
            /**
             * The data
             */
            T item_data;

            /**
             * The result when the item was written
             */
            rc_t item_rc;
        };

        /**
         * The pipe to VPP into which we write the commands
         */
        class CmdQ
        {
        public:
            /**
             * Constructor
             */
            CmdQ();
            /**
             * Destructor
             */
            ~CmdQ();

            /**
             * Copy assignement - only used in UT
             */
            CmdQ& operator=(const CmdQ &f);

            /**
             * Enqueue a command into the Q.
             */
            virtual void enqueue(Cmd *c);
            /**
             * Enqueue a command into the Q.
             */
            virtual void enqueue(std::shared_ptr<Cmd> c);

            /**
             * Enqueue a set of commands
             */
            virtual void enqueue(std::queue<Cmd*> &c);

            /**
             * Write all the commands to HW
             */
            virtual rc_t write();

            /**
             * Blocking Connect to VPP - call once at bootup
             */
            void connect();

            /**
             * Disable the passing of commands to VPP. Whilst disabled all writes
             * will be discarded. Use this during the reset phase.
             */
            void disable();

            /**
             * Enable the passing of commands to VPP - undoes the disable.
             * The Q is enabled by default.
             */
            void enable();

        private:
            /**
             * A queue of enqueued commands, ready to be written
             */
            std::deque<std::shared_ptr<Cmd>> m_queue;

            /**
             * A map of issued, but uncompleted, commands.
             *  i.e. those that we are waiting, async stylee,
             * for VPP to complete
             */
            std::map<Cmd*, std::shared_ptr<Cmd>> m_pending;

            /**
             * VPP Q poll function
             */
            void rx_run();

            /**
             * The thread object running the poll/dispatch/connect thread
             */
            std::unique_ptr<std::thread> m_rx_thread;

            /**
             * The connection to VPP
             */
            Connection m_conn;

            /**
             * A flag for the thread to poll to see if the queue is still alive
             */
            bool m_connected;

            /**
             * A flag indicating the client has disabled the Cmd Q.
             */
            bool m_enabled;
        };

        /**
         * Initialise the HW connection to VPP - the UT version passing
         * a mock Q.
         */
        static void init(CmdQ *f);

        /**
         * Initialise the HW
         */
        static void init();

        /**
         * Enqueue A command for execution
         */
        static void enqueue(Cmd *f);

        /**
         * Enqueue A command for execution
         */
        static void enqueue(std::shared_ptr<Cmd> c);

        /**
         * Enqueue A set of commands for execution
         */
        static void enqueue(std::queue<Cmd*> &c);

        /**
         * Write/Execute all commands hitherto enqueued.
         */
        static rc_t write();

        /**
         * Blocking Connect to VPP
         */
        static void connect();

        /**
         * Blocking pool of the HW connection
         */
        static bool poll();

    private:
        /**
         * The command Q toward HW
         */
        static CmdQ *m_cmdQ;

        /**
         * HW::Item representing the connection state as determined by polling
         */
        static HW::Item<bool> m_poll_state;

        /**
         * Disable the passing of commands to VPP. Whilst disabled all writes
         * will be discarded. Use this during the reset phase.
         */
        static void disable();

        /**
         * Enable the passing of commands to VPP - undoes the disable.
         * The Q is enabled by default.
         */
        static void enable();

        /**
         * Only the OM can enable/disable HW
         */
        friend class OM;

        /**
         * A command pool the HW for liveness
         */
        class Poll: public RpcCmd<HW::Item<bool>, rc_t, vapi::Control_ping>
        {
        public:
            /**
             * Constructor taking the HW::Item to update
             */
            Poll(HW::Item<bool> &item);

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
            bool operator==(const Poll&i) const;
        };
    };
    
    /**
     * bool Specialisation for HW::Item to_string
     */
    template <> std::string HW::Item<bool>::to_string() const;

    /**
     * uint Specialisation for HW::Item to_string
     */
    template <> std::string HW::Item<unsigned int>::to_string() const;
};

#endif
