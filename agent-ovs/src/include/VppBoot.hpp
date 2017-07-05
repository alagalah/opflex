/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VPP_BOOT_H__
#define __VPP_BOOT_H__

namespace VPP
{

    /**
     * When agent starts or re-start, it will sync with
     * VPP current state, by dumping interfaces, bridges,
     * vlans or vxlans and any other configuration in VPP.
     */
    class Boot {
    public:

        Boot();

        void dump();

        void createControlInterface(std::string &interfaceName);

    private:
        bool completed;
        std::shared_ptr<Interface>  controlInterface;

    };
}

#endif
