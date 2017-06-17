/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "VppRenderer.h"
#include "logging.h"

#include <boost/asio/placeholders.hpp>
#include <boost/algorithm/string/split.hpp>

namespace ovsagent {

    using std::bind;
    using opflex::ofcore::OFFramework;
    using boost::property_tree::ptree;
    using boost::asio::placeholders::error;

    VppRenderer::VppRenderer(Agent& agent_):
        Renderer(agent_),
        vppManager(agent_, idGen),
        started(false) {
        LOG(INFO) << "Vpp Renderer";
    }

    VppRenderer::~VppRenderer() {

    }

    void VppRenderer::start() {
        if (started) return;
        started = true;
        vppManager.start(bridgeName);
        vppManager.registerModbListeners();
        LOG(INFO) << "Starting vpp renderer using";
    }

    void VppRenderer::stop() {
        if (!started) return;
        started = false;
        LOG(DEBUG) << "Stopping vpp renderer";

        vppManager.stop();
    }

    Renderer* VppRenderer::create(Agent& agent) {
        return new VppRenderer(agent);
    }

    void VppRenderer::setProperties(const ptree& properties)
    {
        static const std::string VPP_BRIDGE_NAME("opflex-vpp");
        static const std::string ENCAP_VXLAN("encap.vxlan");
        static const std::string ENCAP_IVXLAN("encap.ivxlan");
        static const std::string ENCAP_VLAN("encap.vlan");
        static const std::string UPLINK_IFACE("uplink-iface");
        static const std::string UPLINK_VLAN("uplink-vlan");
        static const std::string ENCAP_IFACE("encap-iface");
        static const std::string REMOTE_IP("remote-ip");
        static const std::string REMOTE_PORT("remote-port");

        bridgeName = properties.get<std::string>(VPP_BRIDGE_NAME, "vpp");

        LOG(INFO) << "Bridge Name " << bridgeName;

        std::string iface;
        auto vxlan = properties.get_child_optional(ENCAP_VXLAN);
        auto vlan = properties.get_child_optional(ENCAP_VLAN);

        if (vlan)
        {
            /* encapType = IntFlowManager::ENCAP_VLAN; */
            /* encapIface = vlan.get().get<std::string>(ENCAP_IFACE, ""); */
            /* count += 1; */
        }
        if (vxlan)
        {
            boost::asio::ip::address remote_ip;
            boost::system::error_code ec;

            remote_ip =
                boost::asio::ip::address::from_string(
                    vxlan.get().get<std::string>(REMOTE_IP, ""));
            if (ec)
            {
                LOG(ERROR) << "Invalid tunnel destination IP: "
                           << vxlan.get().get<std::string>(REMOTE_IP, "")
                           << ": "
                           << ec.message();
            }
            else
            {
                vppManager.uplink().set(vxlan.get().get<std::string>(UPLINK_IFACE, ""),
                                        vxlan.get().get<uint16_t>(UPLINK_VLAN, 0),
                                        vxlan.get().get<std::string>(ENCAP_IFACE, ""),
                                        remote_ip,
                                        vxlan.get().get<uint16_t>(REMOTE_PORT, 4789));
            }
        }
    }
}
