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

#include "VppCmd.hpp"
#include "VppConnection.hpp"

namespace VPP
{
    class HW
    {
    public:
        /**
         * An abstraction of a data item that is written in OM
         */
        template <typename T>
        class Item
        {
        public:
            Item(const T &data):
                item_data(data),
                item_rc(rc_t::NOOP)
             {
             }
            Item():
                item_rc(rc_t::NOOP)
             {
             }
            Item(const T &data,
                 rc_t rc):
                item_data(data),
                item_rc(rc)
             {
             }

            bool operator==(const Item<T> &i) const
            {
                return (item_data == i.item_data);
            }

            Item & operator=(const Item &other)
            {
                item_data = other.item_data;
                item_rc = other.item_rc;
            }

            T &data()
            {
                return (item_data);
            }

            const T &data() const
            {
                return (item_data);
            }

            rc_t rc() const
            {
                 return (item_rc);
            }

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
            CmdQ();
            ~CmdQ();
            CmdQ& operator=(const CmdQ &f);

            virtual void enqueue(Cmd *c);
            virtual void enqueue(std::shared_ptr<Cmd> c);
            virtual rc_t write();

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
            bool m_alive;
        };

        /*
         * Initialise the HW connection to VPP
         */
        static void init(CmdQ *f);
        static void init();

        static void enqueue(Cmd *f);
        static void enqueue(std::shared_ptr<Cmd> c);
        static rc_t write();

        /**
         * Blocking Connect to VPP
         */
        static void connect();

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
        
    private:
        static CmdQ *m_cmdQ;
    };
    
    /*
     * Specialisation for the POD versions of an item
     */
    template <> std::string HW::Item<bool>::to_string() const;
    template <> std::string HW::Item<unsigned int>::to_string() const;
};

#endif
