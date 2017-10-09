/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string>
#include <sstream>
#include <boost/system/error_code.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/ip/host_name.hpp>

//#include <netinet/icmp6.h>

#include <modelgbp/gbp/DirectionEnumT.hpp>
#include <modelgbp/gbp/IntraGroupPolicyEnumT.hpp>
#include <modelgbp/gbp/UnknownFloodModeEnumT.hpp>
#include <modelgbp/gbp/BcastFloodModeEnumT.hpp>
#include <modelgbp/gbp/AddressResModeEnumT.hpp>
#include <modelgbp/gbp/RoutingModeEnumT.hpp>
//#include <modelgbp/gbp/ConnTrackEnumT.hpp>

#include "logging.h"
#include "Endpoint.h"
#include "EndpointManager.h"
#include "VppManager.h"
#include <vom/om.hpp>
#include <vom/interface.hpp>
#include <vom/l2_binding.hpp>
#include <vom/l3_binding.hpp>
#include <vom/bridge_domain.hpp>
#include <vom/bridge_domain_entry.hpp>
#include <vom/bridge_domain_arp_entry.hpp>
#include <vom/interface.hpp>
#include <vom/dhcp_config.hpp>
#include <vom/acl_binding.hpp>
#include <vom/interface_span.hpp>
#include <vom/route_domain.hpp>
#include <vom/route.hpp>
#include <vom/neighbour.hpp>
#include <vom/nat_static.hpp>
#include <vom/nat_binding.hpp>

using std::string;
using std::vector;
using std::ostringstream;
using std::shared_ptr;
using std::unordered_set;
using std::unordered_map;
using std::bind;
using boost::algorithm::trim;
using boost::ref;
using boost::optional;
using boost::asio::deadline_timer;
using boost::asio::ip::address;
using boost::asio::ip::address_v6;
using boost::asio::placeholders::error;
using std::chrono::milliseconds;
using std::unique_lock;
using std::mutex;
using opflex::modb::URI;
using opflex::modb::MAC;
using opflex::modb::class_id_t;

namespace pt = boost::property_tree;
using namespace modelgbp::gbp;
using namespace modelgbp::gbpe;

namespace ovsagent {

    static const char* ID_NAMESPACES[] =
        {"floodDomain", "bridgeDomain", "routingDomain",
         "contract", "externalNetwork"};

    static const char* ID_NMSPC_FD      = ID_NAMESPACES[0];
    static const char* ID_NMSPC_BD      = ID_NAMESPACES[1];
    static const char* ID_NMSPC_RD      = ID_NAMESPACES[2];
    static const char* ID_NMSPC_CON     = ID_NAMESPACES[3];
    static const char* ID_NMSPC_EXTNET  = ID_NAMESPACES[4];

    /**
     * An owner of the objects VPP learns during boot-up
     */
    static const std::string BOOT_KEY = "__boot__";

    VppManager::VppManager(Agent& agent_,
                           IdGenerator& idGen_) :
        agent(agent_),
        taskQueue(agent.getAgentIOService()),
        idGen(idGen_),
        floodScope(FLOOD_DOMAIN),
        virtualDHCPEnabled(false),
        stopping(false) {

        VOM::HW::init();
        VOM::OM::init();
        memset(dhcpMac, 0, sizeof(dhcpMac));

        agent.getFramework().registerPeerStatusListener(this);

    }

    void VppManager::start(const std::string& name) {

        for (size_t i = 0; i < sizeof(ID_NAMESPACES)/sizeof(char*); i++) {
            idGen.initNamespace(ID_NAMESPACES[i]);
        }
        initPlatformConfig();

        /*
         * make sure the first event in the task Q is the blocking
         * connection initiation to VPP ...
         */
        taskQueue.dispatch("init-connection",
                           bind(&VppManager::handleInitConnection, this));

        /**
         * DO BOOT
         */

        /**
         * ... followed by vpp boot dump
         */
        taskQueue.dispatch("boot-dump",
                           bind(&VppManager::handleBoot, this));

        /**
         * ... followed by uplink configuration
         */
        taskQueue.dispatch("uplink-configure",
                           bind(&VppManager::handleUplinkConfigure, this));

    }
    void VppManager::handleInitConnection()
    {
        VOM::HW::connect();

        /**
         * We are insterested in getting interface events from VPP
         */
        std::shared_ptr<VOM::cmd> itf(new VOM::interface::events_cmd(*this));

        VOM::HW::enqueue(itf);
        m_cmds.push_back(itf);

        /**
         * We are insterested in getting DHCP events from VPP
         */
        std::shared_ptr<VOM::cmd> dc(new VOM::dhcp_config::events_cmd(*this));

        VOM::HW::enqueue(dc);
        m_cmds.push_back(dc);

        //VOM::HW::write();

        /**
         * Scehdule a timer to Poll for HW livensss
         */
        m_poll_timer.reset(new deadline_timer(agent.getAgentIOService()));
        m_poll_timer->expires_from_now(boost::posix_time::seconds(3));
        m_poll_timer->async_wait(bind(&VppManager::handleHWPollTimer, this, error));
    }

    void VppManager::handleUplinkConfigure()
    {
        m_uplink.configure(boost::asio::ip::host_name());
    }

    void VppManager::handleSweepTimer(const boost::system::error_code& ec)
    {
        if (ec) return;

        LOG(INFO) << "sweep boot data";

        /*
         * the sweep timer was not cancelled, continue with purging old state.
         */
        VOM::OM::sweep(BOOT_KEY);
    }

    void VppManager::handleHWPollTimer(const boost::system::error_code& ec)
    {
        if (stopping) return;
        if (ec) return;

        if (!VOM::HW::poll())
        {
            /*
             * Lost connection to VPP; reconnect and then replay all the objects
             */
            VOM::HW::connect();
            VOM::OM::replay();
        }

        /*
         * re-scehdule a timer to Poll for HW liveness
         */
        m_poll_timer.reset(new deadline_timer(agent.getAgentIOService()));
        m_poll_timer->expires_from_now(boost::posix_time::seconds(3));
        m_poll_timer->async_wait(bind(&VppManager::handleHWPollTimer, this, error));
    }

    void VppManager::handleBoot()
    {
        /**
         * Read the state from VPP
         */
        VOM::OM::populate(BOOT_KEY);
    }

    void VppManager::registerModbListeners() {
        // Initialize policy listeners
        agent.getEndpointManager().registerListener(this);
        agent.getServiceManager().registerListener(this);
        agent.getExtraConfigManager().registerListener(this);
        agent.getPolicyManager().registerListener(this);
    }

    void VppManager::stop() {
        stopping = true;

        agent.getEndpointManager().unregisterListener(this);
        agent.getServiceManager().unregisterListener(this);
        agent.getExtraConfigManager().unregisterListener(this);
        agent.getPolicyManager().unregisterListener(this);

        if (m_sweep_timer)
        {
            m_sweep_timer->cancel();
        }

        if (m_poll_timer)
        {
            m_poll_timer->cancel();
        }
    }

    void VppManager::setFloodScope(FloodScope fscope) {
        floodScope = fscope;
    }

    void VppManager::setVirtualRouter(bool virtualRouterEnabled,
                                      bool routerAdv,
                                      const string& virtualRouterMac) {
        if (virtualRouterEnabled) {
            try {
                uint8_t routerMac[6];
                MAC(virtualRouterMac).toUIntArray(routerMac);
                m_vr = std::make_shared<VPP::VirtualRouter>(VPP::VirtualRouter(routerMac));
            } catch (std::invalid_argument) {
                LOG(ERROR) << "Invalid virtual router MAC: " << virtualRouterMac;
            }
        }
    }

    void VppManager::setVirtualDHCP(bool dhcpEnabled,
                                    const string& mac) {
        this->virtualDHCPEnabled = dhcpEnabled;
        try {
            MAC(mac).toUIntArray(dhcpMac);
        } catch (std::invalid_argument) {
            LOG(ERROR) << "Invalid virtual DHCP server MAC: " << mac;
        }
    }


    void VppManager::setMulticastGroupFile(const std::string& mcastGroupFile) {
        this->mcastGroupFile = mcastGroupFile;
    }

    void VppManager::enableConnTrack() {
        conntrackEnabled = true;
    }

    void VppManager::endpointUpdated(const std::string& uuid) {
        if (stopping) return;

        taskQueue.dispatch(uuid,
                           bind(&VppManager::handleEndpointUpdate, this, uuid));
    }

    void VppManager::serviceUpdated(const std::string& uuid) {
        if (stopping) return;

        taskQueue.dispatch(uuid,
                           bind(&VppManager::handleAnycastServiceUpdate,
                                this, uuid));
    }

    void VppManager::rdConfigUpdated(const opflex::modb::URI& rdURI) {
        domainUpdated(RoutingDomain::CLASS_ID, rdURI);
    }

    void VppManager::egDomainUpdated(const opflex::modb::URI& egURI) {
        if (stopping) return;

        taskQueue.dispatch(egURI.toString(),
                           bind(&VppManager::handleEndpointGroupDomainUpdate,
                                this, egURI));
    }

    void VppManager::domainUpdated(class_id_t cid, const URI& domURI) {
        if (stopping) return;

        taskQueue.dispatch(domURI.toString(),
                           bind(&VppManager::handleDomainUpdate,
                                this, cid, domURI));
    }

    void VppManager::contractUpdated(const opflex::modb::URI& contractURI) {
        if (stopping) return;
        taskQueue.dispatch(contractURI.toString(),
                           bind(&VppManager::handleContractUpdate,
                                this, contractURI));
    }

    void VppManager::handle_interface_event(VOM::interface::events_cmd *e)
    {
        if (stopping) return;
        taskQueue.dispatch("InterfaceEvent",
                           bind(&VppManager::handleInterfaceEvent,
                                this, e));
    }

    void VppManager::handle_interface_stat(VOM::interface::stats_cmd *e)
    {
        if (stopping) return;
        taskQueue.dispatch("InterfaceStat",
                           bind(&VppManager::handleInterfaceStat,
                                this, e));
    }

    void VppManager::handle_dhcp_event(VOM::dhcp_config::events_cmd *e)
    {
        if (stopping) return;
        taskQueue.dispatch("dhcp-config-event",
                           bind(&VppManager::handleDhcpEvent,
                                this, e));
    }

    void VppManager::configUpdated(const opflex::modb::URI& configURI) {
        if (stopping) return;
        agent.getAgentIOService()
            .dispatch(bind(&VppManager::handleConfigUpdate, this, configURI));
    }

    void VppManager::portStatusUpdate(const string& portName,
                                      uint32_t portNo, bool fromDesc) {
        if (stopping) return;
        agent.getAgentIOService()
            .dispatch(bind(&VppManager::handlePortStatusUpdate, this,
                           portName, portNo));
    }

    void VppManager::peerStatusUpdated(const std::string&, int,
                                       PeerStatus peerStatus) {
        if (stopping || isSyncing) return;
    }

    bool VppManager::getGroupForwardingInfo(const URI& epgURI, uint32_t& vnid,
                                            optional<URI>& rdURI,
                                            uint32_t& rdId,
                                            optional<URI>& bdURI,
                                            uint32_t& bdId,
                                            optional<URI>& fdURI,
                                            uint32_t& fdId) {
        PolicyManager& polMgr = agent.getPolicyManager();
        optional<uint32_t> epgVnid = polMgr.getVnidForGroup(epgURI);
        if (!epgVnid) {
            return false;
        }
        vnid = epgVnid.get();

        optional<shared_ptr<RoutingDomain> > epgRd = polMgr.getRDForGroup(epgURI);
        optional<shared_ptr<BridgeDomain> > epgBd = polMgr.getBDForGroup(epgURI);
        optional<shared_ptr<FloodDomain> > epgFd = polMgr.getFDForGroup(epgURI);
        if (!epgRd && !epgBd && !epgFd) {
            return false;
        }

        bdId = 0;
        if (epgBd) {
            bdURI = epgBd.get()->getURI();
            bdId = getId(BridgeDomain::CLASS_ID, bdURI.get());
        }
        fdId = 0;
        if (epgFd) {    // FD present -> flooding is desired
            if (floodScope == ENDPOINT_GROUP) {
                fdURI = epgURI;
            } else  {
                fdURI = epgFd.get()->getURI();
            }
            fdId = getId(FloodDomain::CLASS_ID, fdURI.get());
        }
        rdId = 0;
        if (epgRd) {
            rdURI = epgRd.get()->getURI();
            rdId = getId(RoutingDomain::CLASS_ID, rdURI.get());
        }
        return true;
    }

void VppManager::handleEndpointUpdate(const string& uuid) {

    LOG(DEBUG) << "stub: Updating endpoint " << uuid;

    EndpointManager& epMgr = agent.getEndpointManager();
    shared_ptr<const Endpoint> epWrapper = epMgr.getEndpoint(uuid);

    if (!epWrapper) {
        /*
         * remove everything related to this endpoint
         */
        VOM::OM::remove(uuid);
        return;
    }

    optional<URI> epgURI = epMgr.getComputedEPG(uuid);
    if (!epgURI) {      // can't do much without EPG
        return;
    }

    /*
     * This is an update to all the state related to this endpoint.
     * At the end of processing we want all the state realted to this endpint,
     * that we don't touch here, gone.
     */
    VOM::OM::mark(uuid);

    const Endpoint& endPoint = *epWrapper.get();
    const optional<string>& vppInterfaceName = endPoint.getInterfaceName();
    int rv;

    uint32_t epgVnid, rdId, bdId, fgrpId;
    optional<URI> fgrpURI, bdURI, rdURI;
    if (!getGroupForwardingInfo(epgURI.get(), epgVnid, rdURI,
                            rdId, bdURI, bdId, fgrpURI, fgrpId)) {
        return;
    }

    /*
     * the route-domain the endpoint is in
     */
    route_domain rd(rdId);
    VOM::OM::write(uuid, rd);

    /*
     * We want a veth interface - admin up
     */
    VOM::interface itf(vppInterfaceName.get(),
                       VOM::interface::type_t::AFPACKET,
                       VOM::interface::admin_state_t::UP,
                       rd);
    VOM::OM::write(uuid, itf);

    VOM::interface itftap("tap-"+vppInterfaceName.get(),
                       VOM::interface::type_t::TAP,
                       VOM::interface::admin_state_t::UP);
    VOM::OM::write(uuid, itftap);

    VOM::interface_span itfSpan(itf, itftap, VOM::interface_span::state_t::RX_ENABLED);
    VOM::OM::write(uuid, itfSpan);

    uint8_t macAddr[6];
    bool hasMac = endPoint.getMAC() != boost::none;

    if (hasMac)
        endPoint.getMAC().get().toUIntArray(macAddr);

    /* check and parse the IP-addresses */
    boost::system::error_code ec;

    std::vector<address> ipAddresses;
    for (const string& ipStr : endPoint.getIPs()) {
        address addr = address::from_string(ipStr, ec);
        if (ec) {
            LOG(WARNING) << "Invalid endpoint IP: "
                         << ipStr << ": " << ec.message();
        } else {
            ipAddresses.push_back(addr);
        }
    }

    if (hasMac) {
        address_v6 linkLocalIp(network::construct_link_local_ip_addr(macAddr));
        if (endPoint.getIPs().find(linkLocalIp.to_string()) ==
            endPoint.getIPs().end())
            ipAddresses.push_back(linkLocalIp);
    }

    VOM::ACL::l2_list::rules_t rules;
    if (itf.handle().value()) {
        if (endPoint.isPromiscuousMode()) {
            VOM::ACL::l2_rule rulev6(50,
                                     VOM::ACL::action_t::PERMIT,
                                     VOM::route::prefix_t::ZEROv6,
                                     macAddr,
                                     VOM::mac_address_t::ZERO);

            VOM::ACL::l2_rule rulev4(51,
                                     VOM::ACL::action_t::PERMIT,
                                     VOM::route::prefix_t::ZERO,
                                     macAddr,
                                     VOM::mac_address_t::ZERO);
            rules.insert(rulev4);
            rules.insert(rulev6);
        } else if (hasMac) {
            VOM::ACL::l2_rule rulev6(20,
                                     VOM::ACL::action_t::PERMIT,
                                     VOM::route::prefix_t::ZEROv6,
                                     macAddr,
                                     VOM::mac_address_t::ONE);

            VOM::ACL::l2_rule rulev4(21,
                                     VOM::ACL::action_t::PERMIT,
                                     VOM::route::prefix_t::ZERO,
                                     macAddr,
                                     VOM::mac_address_t::ONE);
            rules.insert(rulev4);
            rules.insert(rulev6);

            for (const address& ipAddr : ipAddresses) {
                // Allow IPv4/IPv6 packets from port with EP IP address
                VOM::route::prefix_t pfx(ipAddr, ipAddr.is_v4() ? 32 : 128);
                if (ipAddr.is_v6()) {
                    VOM::ACL::l2_rule rule(30,
                                           VOM::ACL::action_t::PERMIT,
                                           pfx,
                                           macAddr,
                                           VOM::mac_address_t::ONE);
                    rules.insert(rule);
                } else {
                    VOM::ACL::l2_rule rule(31,
                                           VOM::ACL::action_t::PERMIT,
                                           pfx,
                                           macAddr,
                                           VOM::mac_address_t::ONE);
                    rules.insert(rule);
                }
            }
        }

        for (const Endpoint::virt_ip_t& vip : endPoint.getVirtualIPs()) {
            network::cidr_t vip_cidr;
            if (!network::cidr_from_string(vip.second, vip_cidr)) {
                LOG(WARNING) << "Invalid endpoint VIP (CIDR): " << vip.second;
                continue;
            }
            uint8_t vmac[6];
            vip.first.toUIntArray(vmac);

            for (const address& ipAddr : ipAddresses) {
                if (!network::cidr_contains(vip_cidr, ipAddr)) {
                    continue;
                }
                VOM::route::prefix_t pfx(ipAddr, ipAddr.is_v4() ? 32 : 128);
                if (ipAddr.is_v6()) {
                    VOM::ACL::l2_rule rule(60,
                                           VOM::ACL::action_t::PERMIT,
                                           pfx,
                                           vmac,
                                           VOM::mac_address_t::ONE);
                    rules.insert(rule);
                } else {
                    VOM::ACL::l2_rule rule(61,
                                           VOM::ACL::action_t::PERMIT,
                                           pfx,
                                           vmac,
                                           VOM::mac_address_t::ONE);
                    rules.insert(rule);
                }
            }
        }

        VOM::ACL::l2_list acl(uuid, rules);
        VOM::OM::write(uuid, acl);

        VOM::ACL::l2_binding binding(VOM::direction_t::INPUT, itf, acl);
        VOM::OM::write(uuid, binding);
    }

    optional<shared_ptr<FloodDomain> > fd =
        agent.getPolicyManager().getFDForGroup(epgURI.get());

    uint8_t arpMode = AddressResModeEnumT::CONST_UNICAST;
    uint8_t ndMode = AddressResModeEnumT::CONST_UNICAST;
    uint8_t unkFloodMode = UnknownFloodModeEnumT::CONST_DROP;
    uint8_t bcastFloodMode = BcastFloodModeEnumT::CONST_NORMAL;
    if (fd) {
        // alagalah Irrespective of flooding scope (epg vs. flood-domain), the
        // properties of the flood-domain object decide how flooding
        // is done.

        arpMode = fd.get()
            ->getArpMode(AddressResModeEnumT::CONST_UNICAST);
        ndMode = fd.get()
            ->getNeighborDiscMode(AddressResModeEnumT::CONST_UNICAST);
        unkFloodMode = fd.get()
            ->getUnknownFloodMode(UnknownFloodModeEnumT::CONST_DROP);
        bcastFloodMode = fd.get()
            ->getBcastFloodMode(BcastFloodModeEnumT::CONST_NORMAL);

        VOM::bridge_domain bd(fgrpId);

        if (VOM::rc_t::OK != VOM::OM::write(uuid, bd))
        {
            LOG(ERROR) << "VppApi did not create bridge: "
                       << *fd.get()->getName()
                       << " for port: "
                       << vppInterfaceName.get();
            return;
        }

        VOM::l2_binding l2itf(itf, bd);

        if (VOM::rc_t::OK != VOM::OM::write(uuid, l2itf))
        {
            LOG(ERROR) << "VppApi did not set bridge: "
                       << *fd.get()->getName()
                       << " for port: "
                       << vppInterfaceName.get();
            return;
        }

        if (hasMac)
        {
            /*
             * An entry in the BD's L2 FIB to forward traffic to the end-point
             */
            VOM::bridge_domain_entry be(bd, {macAddr}, itf);

            if (VOM::rc_t::OK != VOM::OM::write(uuid, be)) {
                LOG(ERROR) << "bridge-domain-entry: "
                           << vppInterfaceName.get();
                return;
            }

            /*
             * An entry in the BD's ARP termination table to reply to
             * ARP request for this end-point from other local end-points
             */
            for (const address& ipAddr : ipAddresses) {
                VOM::bridge_domain_arp_entry bae(bd, {macAddr}, ipAddr);

                if (VOM::rc_t::OK != VOM::OM::write(uuid, bae)) {
                    LOG(ERROR) << "bridge-domain-arp-entry: "
                               << vppInterfaceName.get();
                    return;
                }

                /*
                 * Add an L3 rewrite route to the end point. This will match packets
                 * from locally attached EPs in different subnets.
                 */
                VOM::route::path ep_path(ipAddr, itf);
                VOM::route::ip_route ep_route(rd, ipAddr);
                ep_route.add(ep_path);
                VOM::OM::write(uuid, ep_route);

                /*
                 * Add a neighbour entry
                 */
                VOM::neighbour ne(itf, {macAddr}, ipAddr);
                VOM::OM::write(uuid, ne);
            }

            /*
             * If the virtual router is enabled, add Floating IP mappings, a.k.a. NAT
             */
            uint8_t routingMode =
                agent.getPolicyManager().getEffectiveRoutingMode(epgURI.get());

            if (m_vr && routingMode == RoutingModeEnumT::CONST_ENABLED) {
                // IP address mappings
                for(const Endpoint::IPAddressMapping& ipm :
                        endPoint.getIPAddressMappings()) {
                    if (!ipm.getMappedIP() || !ipm.getEgURI())
                        continue;

                    address mappedIp =
                        address::from_string(ipm.getMappedIP().get(), ec);
                    if (ec) continue;

                    address floatingIp;
                    if (ipm.getFloatingIP()) {
                        floatingIp =
                            address::from_string(ipm.getFloatingIP().get(), ec);
                        if (ec) continue;
                        if (floatingIp.is_v4() != mappedIp.is_v4()) continue;

                        if (floatingIp.is_v4()) {
                            /*
                             * A static binding of the inside (mapped) to outside (floating)
                             */
                            nat_static ns(rd, mappedIp, floatingIp.to_v4());
                            VOM::OM::write(uuid, ns);

                            /*
                             * the VM's interface is the inside
                             */
                            nat_binding nb_in(itf, direction_t::INPUT,
                                              l3_proto_t::IPV4,
                                              nat_binding::zone_t::INSIDE);
                            VOM::OM::write(uuid, nb_in);

                            /*
                             * and the BVI in the BD is the outside.
                             */
                            VOM::interface bvi("bvi-" + std::to_string(bd.id()),
                                               VOM::interface::type_t::BVI,
                                               VOM::interface::admin_state_t::UP,
                                               rd);
                            VOM::OM::write(uuid, bvi);

                            nat_binding nb_out(itf, direction_t::INPUT,
                                               l3_proto_t::IPV4,
                                               nat_binding::zone_t::OUTSIDE);
                            VOM::OM::write(uuid, nb_out);
                        }
                    }
                }
            }
        }
    }

    /*
     * That's all folks ...
     */
    VOM::OM::sweep(uuid);
}

    void VppManager::handleAnycastServiceUpdate(const string& uuid) {
        LOG(DEBUG) << "Updating anycast service " << uuid;

    }

    void VppManager::updateEPGFlood(const URI& epgURI, uint32_t epgVnid,
                                    uint32_t fgrpId, address epgTunDst) {
        LOG(DEBUG) << "Updating EPGFlood " << fgrpId;
    }


    void VppManager::handleEndpointGroupDomainUpdate(const URI& epgURI)
    {
        LOG(DEBUG) << "Updating endpoint-group " << epgURI;

        const string& epg_uuid = epgURI.toString();
        PolicyManager &pm = agent.getPolicyManager();

        if (!agent.getPolicyManager().groupExists(epgURI))
        {
            VOM::OM::remove(epg_uuid);
            return;
        }
        uint32_t epgVnid, rdId, bdId, fgrpId;
        optional<URI> fgrpURI, bdURI, rdURI;
        if (!getGroupForwardingInfo(epgURI, epgVnid, rdURI,
                                    rdId, bdURI, bdId, fgrpURI, fgrpId)) {
            return;
        }

        /*
         * Mark all of this EPG's state stale.
         */
        VOM::OM::mark(epg_uuid);

        /*
         * Construct the BridgeDomain
         */
        VOM::bridge_domain bd(fgrpId);

        VOM::OM::write(epg_uuid, bd);

        /*
         * Construct the encap-link
         */
        std::shared_ptr<VOM::interface> encap_link(m_uplink.mk_interface(epg_uuid, epgVnid));

        /*
         * Add the encap-link to the BD
         *
         * If the encap link is a VLAN, then set the pop VTR operation on the
         * link so that the VLAN tag is correctly pop/pushed on rx/tx resp.
         */
        VOM::l2_binding l2(*encap_link, bd);
        if (VOM::interface::type_t::VXLAN != encap_link->type()) {
            l2.set(l2_binding::l2_vtr_op_t::L2_VTR_POP_1, epgVnid);
        }
        VOM::OM::write(epg_uuid, l2);

        /*
         * Add the BVIs to the BD
         */
        optional<shared_ptr<RoutingDomain>> epgRd = pm.getRDForGroup(epgURI);

        VOM::route_domain rd(rdId);
        VOM::OM::write(epg_uuid, rd);

        updateBVIs(epgURI, bd, rd, encap_link);

        /*
         * Sweep the remaining EPG's state
         */
        VOM::OM::sweep(epg_uuid);
    }

    void VppManager::updateBVIs(const URI& epgURI,
                                VOM::bridge_domain &bd,
                                const VOM::route_domain &rd,
                                std::shared_ptr<VOM::interface> encap_link)
    {
        LOG(DEBUG) << "Updating BVIs";

        const string& epg_uuid = epgURI.toString();
        PolicyManager::subnet_vector_t subnets;
        agent.getPolicyManager().getSubnetsForGroup(epgURI, subnets);

        /*
         * Create a BVI interface for the EPG and add it to the bridge-domain
         */
        VOM::interface bvi("bvi-" + std::to_string(bd.id()),
                           VOM::interface::type_t::BVI,
                           VOM::interface::admin_state_t::UP,
                           rd);
        if (m_vr)
        {
            /*
             * Set the BVI's MAC address to the Virtual Router
             * address, so packets destined to the VR are handled
             * by layer 3.
             */
            bvi.set(m_vr->mac());
        }
        VOM::OM::write(epg_uuid, bvi);

        VOM::l2_binding l2(bvi, bd);
        VOM::OM::write(epg_uuid, l2);

        for (shared_ptr<Subnet>& sn : subnets)
        {
            optional<address> routerIp =
                PolicyManager::getRouterIpForSubnet(*sn);

            if (routerIp)
            {
                boost::asio::ip::address raddr = routerIp.get();
                /*
                 * apply the host prefix on the BVI
                 * and add an entry into the ARP Table for it.
                 */
                VOM::route::prefix_t host_pfx(raddr);
                VOM::l3_binding l3(bvi, host_pfx);
                VOM::OM::write(epg_uuid, l3);

                VOM::bridge_domain_arp_entry bae(bd,
                                                 bvi.l2_address().to_mac(),
                                                 raddr);
                VOM::OM::write(epg_uuid, bae);

                /*
                 * add the subnet as a DVR route, so all other EPs will be
                 * L3-bridged to the uplink
                 */
                VOM::route::prefix_t subnet_pfx(raddr, sn->getPrefixLen().get());
                VOM::route::path dvr_path(*encap_link,
                                          VOM::nh_proto_t::ETHERNET);
                VOM::route::ip_route subnet_route(rd, subnet_pfx);
                subnet_route.add(dvr_path);
                VOM::OM::write(epg_uuid, subnet_route);
            }
        }
    }

    void VppManager::updateGroupSubnets(const URI& egURI, uint32_t bdId,
                                        uint32_t rdId) {
        LOG(DEBUG) << "Updating GroupSubnets bd: " << bdId << " rd: " << rdId;
    }

    void VppManager::handleRoutingDomainUpdate(const URI& rdURI) {
        optional<shared_ptr<RoutingDomain > > rd =
            RoutingDomain::resolve(agent.getFramework(), rdURI);

        if (!rd) {
            LOG(DEBUG) << "Cleaning up for RD: " << rdURI;
            idGen.erase(getIdNamespace(RoutingDomain::CLASS_ID), rdURI.toString());
            return;
        }
        LOG(DEBUG) << "Updating routing domain " << rdURI;
    }

    void
    VppManager::handleDomainUpdate(class_id_t cid, const URI& domURI) {

        switch (cid) {
        case RoutingDomain::CLASS_ID:
            handleRoutingDomainUpdate(domURI);
            break;
        case Subnet::CLASS_ID:
            if (!Subnet::resolve(agent.getFramework(), domURI)) {
                LOG(DEBUG) << "Cleaning up for Subnet: " << domURI;
            }
            break;
        case BridgeDomain::CLASS_ID:
            if (!BridgeDomain::resolve(agent.getFramework(), domURI)) {
                LOG(DEBUG) << "Cleaning up for BD: " << domURI;
                idGen.erase(getIdNamespace(cid), domURI.toString());
            }
            break;
        case FloodDomain::CLASS_ID:
            if (!FloodDomain::resolve(agent.getFramework(), domURI)) {
                LOG(DEBUG) << "Cleaning up for FD: " << domURI;
                idGen.erase(getIdNamespace(cid), domURI.toString());
            }
            break;
        case FloodContext::CLASS_ID:
            if (!FloodContext::resolve(agent.getFramework(), domURI)) {
                LOG(DEBUG) << "Cleaning up for FloodContext: " << domURI;
                if (removeFromMulticastList(domURI))
                    multicastGroupsUpdated();
            }
            break;
        case L3ExternalNetwork::CLASS_ID:
            if (!L3ExternalNetwork::resolve(agent.getFramework(), domURI)) {
                LOG(DEBUG) << "Cleaning up for L3ExtNet: " << domURI;
                idGen.erase(getIdNamespace(cid), domURI.toString());
            }
            break;
        }
        LOG(DEBUG) << "Updating domain " << domURI;
    }

    void
    VppManager::handleInterfaceEvent(VOM::interface::events_cmd *e)
    {
        LOG(DEBUG) << "Interface Event: " << *e;

        std::lock_guard<VOM::interface::events_cmd> lg(*e);

        for (auto &msg : *e)
        {
            auto &payload = msg.get_payload();

            VOM::handle_t handle(payload.sw_if_index);
            std::shared_ptr<VOM::interface> sp = VOM::interface::find(handle);

            if (sp)
            {
                VOM::interface::oper_state_t oper_state =
                    VOM::interface::oper_state_t::from_int(payload.link_up_down);

                LOG(DEBUG) << "Interface Event: " << sp->to_string()
                           << " state: " << oper_state.to_string();

                sp->set(oper_state);
            }
        }

        e->flush();
    }

    void
    VppManager::handleInterfaceStat(VOM::interface::stats_cmd *e)
    {
        LOG(DEBUG) << "Interface Stat: " << *e;

        std::lock_guard<VOM::interface::stats_cmd> lg(*e);

        for (auto &msg : *e)
            {
                auto &payload = msg.get_payload();

                for (int i=0; i < payload.count; ++i) {
                    auto &data = payload.data[i];

                    VOM::handle_t handle(data.sw_if_index);
                    std::shared_ptr<VOM::interface> sp = VOM::interface::find(handle);
                    LOG(INFO) << "Interface Stat: " << sp->to_string()
                               << " stat rx_packets: " << data.rx_packets
                              << " stat rx_bytes: " << data.rx_bytes
                              << " stat tx_packets: " << data.tx_packets
                              << " stat tx_bytes: " << data.tx_bytes;
                }
            }

        e->flush();
    }

    void
    VppManager::handleDhcpEvent(VOM::dhcp_config::events_cmd *e)
    {
        LOG(INFO) << "DHCP Event: " << *e;
        m_uplink.handle_dhcp_event(e);
    }

    void
    VppManager::updateEndpointFloodGroup(const opflex::modb::URI& fgrpURI,
                                         const Endpoint& endPoint, uint32_t epPort,
                                         bool isPromiscuous,
                                         optional<shared_ptr<FloodDomain> >& fd) {
        LOG(DEBUG) << "Updating domain " << fgrpURI;
    }

    void VppManager::removeEndpointFromFloodGroup(const std::string& epUUID) {
        /*
         * Remove the endpoint from the flood group (Bridge in VPP)
         * Remove any configurations and flows from VPP
         */
        LOG(DEBUG) << "Removing EP from FD " << epUUID;
    }

    void
    VppManager::handleContractUpdate(const opflex::modb::URI& contractURI) {
        LOG(DEBUG) << "Updating contract " << contractURI;

        const string& contractId = contractURI.toString();
        PolicyManager& polMgr = agent.getPolicyManager();
        if (!polMgr.contractExists(contractURI)) {  // Contract removed
            idGen.erase(getIdNamespace(Contract::CLASS_ID), contractURI.toString());
            return;
        }
    }

    void VppManager::initPlatformConfig() {

        using namespace modelgbp::platform;

        optional<shared_ptr<Config> > config =
            Config::resolve(agent.getFramework(),
                            agent.getPolicyManager().getOpflexDomain());
    }

    void VppManager::handleConfigUpdate(const opflex::modb::URI& configURI) {
        LOG(DEBUG) << "Updating platform config " << configURI;

        initPlatformConfig();

        /**
         * Now that we are known to be opflex connected,
         * Scehdule a timer to sweep the state we read when we first connected
         * to VPP.
         */
        m_sweep_timer.reset(new deadline_timer(agent.getAgentIOService()));
        m_sweep_timer->expires_from_now(boost::posix_time::seconds(30));
        m_sweep_timer->async_wait(bind(&VppManager::handleSweepTimer, this, error));
    }

    void VppManager::handlePortStatusUpdate(const string& portName,
                                            uint32_t) {
        LOG(DEBUG) << "Port-status update for " << portName;
    }

    void VppManager::getGroupVnidAndRdId(const unordered_set<URI>& uris,
                                         /* out */unordered_map<uint32_t, uint32_t>& ids) {
        PolicyManager& pm = agent.getPolicyManager();
        for (const URI& u : uris) {
            optional<uint32_t> vnid = pm.getVnidForGroup(u);
            optional<shared_ptr<RoutingDomain> > rd;
            if (vnid) {
                rd = pm.getRDForGroup(u);
            } else {
                rd = pm.getRDForL3ExtNet(u);
                if (rd) {
                    vnid = getExtNetVnid(u);
                }
            }
            if (vnid && rd) {
                ids[vnid.get()] = getId(RoutingDomain::CLASS_ID,
                                        rd.get()->getURI());
            }
        }
    }

    typedef std::function<bool(opflex::ofcore::OFFramework&,
                               const string&,
                               const string&)> IdCb;

    static const IdCb ID_NAMESPACE_CB[] =
        {IdGenerator::uriIdGarbageCb<FloodDomain>,
         IdGenerator::uriIdGarbageCb<BridgeDomain>,
         IdGenerator::uriIdGarbageCb<RoutingDomain>,
         IdGenerator::uriIdGarbageCb<Contract>,
         IdGenerator::uriIdGarbageCb<L3ExternalNetwork>};


    const char * VppManager::getIdNamespace(class_id_t cid) {
        const char *nmspc = NULL;
        switch (cid) {
        case RoutingDomain::CLASS_ID:   nmspc = ID_NMSPC_RD; break;
        case BridgeDomain::CLASS_ID:    nmspc = ID_NMSPC_BD; break;
        case FloodDomain::CLASS_ID:     nmspc = ID_NMSPC_FD; break;
        case Contract::CLASS_ID:        nmspc = ID_NMSPC_CON; break;
        case L3ExternalNetwork::CLASS_ID: nmspc = ID_NMSPC_EXTNET; break;
        default:
            assert(false);
        }
        return nmspc;
    }

    uint32_t VppManager::getId(class_id_t cid, const URI& uri) {
        return idGen.getId(getIdNamespace(cid), uri.toString());
    }

    uint32_t VppManager::getExtNetVnid(const opflex::modb::URI& uri) {
        // External networks are assigned private VNIDs that have bit 31 (MSB)
        // set to 1. This is fine because legal VNIDs are 24-bits or less.
        return (getId(L3ExternalNetwork::CLASS_ID, uri) | (1 << 31));
    }

    void VppManager::updateMulticastList(const optional<string>& mcastIp,
                                         const URI& uri) {
    }

    bool VppManager::removeFromMulticastList(const URI& uri) {
        return true;
    }

    static const std::string MCAST_QUEUE_ITEM("mcast-groups");

    void VppManager::multicastGroupsUpdated() {
        taskQueue.dispatch(MCAST_QUEUE_ITEM,
                           bind(&VppManager::writeMulticastGroups, this));
    }

    void VppManager::writeMulticastGroups() {
        if (mcastGroupFile == "") return;

        pt::ptree tree;
        pt::ptree groups;
        for (MulticastMap::value_type& kv : mcastMap)
            groups.push_back(std::make_pair("", pt::ptree(kv.first)));
        tree.add_child("multicast-groups", groups);

        try {
            pt::write_json(mcastGroupFile, tree);
        } catch (pt::json_parser_error e) {
            LOG(ERROR) << "Could not write multicast group file "
                       << e.what();
        }
    }

    VPP::Uplink &VppManager::uplink()
    {
        return m_uplink;
    }

} // namespace ovsagent
