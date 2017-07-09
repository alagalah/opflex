/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <boost/algorithm/string.hpp>

#include "VppInterface.hpp"
#include "VppTapInterface.hpp"
#include "VppSubInterface.hpp"

using namespace VPP;

std::unique_ptr<Interface>
Interface::new_interface(const vapi_payload_sw_interface_details &vd)
{
    std::unique_ptr<Interface> up_itf;

    /**
     * Determine the Interface type from the name and VLAN attributes
     */
    std::string name = reinterpret_cast<const char*>(vd.interface_name);
    type_t type = Interface::type_t::from_string(name);
    admin_state_t state = Interface::admin_state_t::from_int(vd.link_up_down);
    handle_t hdl(vd.sw_if_index);

    if (type_t::AFPACKET == type)
    {
        /*
         * need to strip VPP's "host-" prefix from the interface name
         */
        name = name.substr(5);
    }

    /*
     * pull out the other special cases
     */
    if (type_t::TAP == type)
    {
        /*
         * TAP interface
         */
        up_itf.reset(new TapInterface(hdl, name, state, Route::prefix_t()));
    }
    else if ((name.find(".") != std::string::npos) &&
             (0 != vd.sub_id))
    {
        /*
         * Sub-interface
         *   split the name into the parent and VLAN
         */
        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("."));

        Interface parent(parts[0], type, state);
        up_itf.reset(new SubInterface(hdl, parent, state, vd.sub_id));
    }
    else
    {
        up_itf.reset(new Interface(hdl, name, type, state));
    }

    return (up_itf);
}
