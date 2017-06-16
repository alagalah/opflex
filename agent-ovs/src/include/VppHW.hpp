/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_EXECTUOR_H__
#define __VPP_EXECTUOR_H__

#include "VppCmd.hpp"
#include "VppConnection.h"

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

            virtual void enqueue(Cmd *f);
            virtual rc_t write();
        private:
            std::unique_ptr<ovsagent::VppConnection> m_conn;
        };

        /*
         * Initialise the HW connection to VPP
         */
        static void init(CmdQ *f);
        static void init();

        static void enqueue(Cmd *f);
        static rc_t write();

    private:
        static CmdQ *m_fifo;
    };
    
    /*
     * Specialisation for the boolean version of an item
     */
    template <> std::string HW::Item<bool>::to_string() const;
};

#endif
