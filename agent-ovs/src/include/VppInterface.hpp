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
#include "VppCmd.hpp"
#include "VppInstDB.hpp"
#include "VppEnum.hpp"
#include "VppRoute.hpp"

namespace VPP
{
    /**
     * A base class for all Object in the VPP Object-Model.
     *  provides the abstract interface.
     */
    class Interface: public Object
    {
    public:
        /**
         * The name of a nameless interface
         */
        const static std::string NAMELESS;

        /**
         * An interface type
         */
        struct type_t: Enum<type_t>
        {
            type_t(int v, const std::string &s);

            const static type_t VHOST;
            const static type_t BVI;
        };

        /**
         * The admin state of the interface
         */
        struct admin_state_t: Enum<admin_state_t>
        {
            admin_state_t(int v, const std::string &s);
            
            const static admin_state_t DOWN;
            const static admin_state_t UP;
        };

        /**
         * Construct a new object matching the desried state
         */
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state);
        Interface(const std::string &name,
                  type_t type,
                  admin_state_t state,
                  Route::prefix_t prefix);
        Interface(type_t type,
                  admin_state_t state,
                  Route::prefix_t prefix);
        ~Interface();
        Interface(const Interface& o);

        /**
         * Debug rpint function
         */
        std::string to_string(void);

        /**
         * Return VPP's handle to this object
         */
        const handle_t & handle() const;

        /**
         * Return the interface type
         */
        const type_t & type() const;

        /**
         * A functor class that creates an interface
         */
        class CreateCmd: public CmdT<HW::Item<handle_t> >
        {
        public:
            CreateCmd(HW::Item<handle_t> &item,
                       const std::string &name,
                       type_t type);
            CreateCmd(HW::Item<handle_t> &item,
                       type_t type);

            rc_t exec();
            std::string to_string();

            bool operator==(const CreateCmd&i) const;
        private:
            const std::string &m_name;
            type_t m_type;
        };

        /**
         * A cmd class that Delete an interface
         */
        class DeleteCmd: public CmdT<HW::Item<handle_t> >
        {
        public:
            DeleteCmd(HW::Item<handle_t> &item,
                       type_t type);

            rc_t exec();
            std::string to_string();

            bool operator==(const DeleteCmd&i) const;
        private:
            type_t m_type;
        };

        /**
         * A cmd class that changes the admin state
         */
        class StateChangeCmd: public CmdT<HW::Item<admin_state_t> >
        {
        public:
            StateChangeCmd(HW::Item<admin_state_t> &s,
                            const HW::Item<handle_t> &h);

            rc_t exec();
            std::string to_string();

            bool operator==(const StateChangeCmd&i) const;
        private:
            const HW::Item<handle_t> &m_hdl;
        };
        
        /**
         * A functor class that creates an interface
         */
        class PrefixAddCmd: public CmdT<HW::Item<Route::prefix_t> >
        {
        public:
            PrefixAddCmd(HW::Item<Route::prefix_t> &item,
                         const HW::Item<handle_t> &h);

            rc_t exec();
            std::string to_string();

            bool operator==(const PrefixAddCmd&i) const;
        private:
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * A functor class that creates an interface
         */
        class PrefixDelCmd: public CmdT<HW::Item<Route::prefix_t> >
        {
        public:
            PrefixDelCmd(HW::Item<Route::prefix_t> &item,
                         const HW::Item<handle_t> &h);

            rc_t exec();
            std::string to_string();

            bool operator==(const PrefixDelCmd&i) const;
        private:
            const HW::Item<handle_t> &m_hdl;
        };

        /**
         * The the instance of the Interface in the Object-Model
         */
        static std::shared_ptr<Interface> find(const Interface &temp);

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const Interface &obj);

        static std::shared_ptr<Interface> find_or_add(const Interface &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VPP::OM;
    
        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * The interfaces name
         */
        const std::string m_name;
    
        /**
         * The SW interface handle VPP has asigned to the interface
         */
        HW::Item<handle_t> m_hdl;

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
         * An IP prefix assigned to the interface
         */
        HW::Item<Route::prefix_t> m_prefix;

        /**
         * A map of all interfaces key against the interface's name
         */
        static InstDB<const std::string, Interface> m_db;
    };
};
#endif
