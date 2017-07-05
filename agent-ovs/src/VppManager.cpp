/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <boost/system/error_code.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/asio/placeholders.hpp>

#include <netinet/icmp6.h>

#include <modelgbp/gbp/DirectionEnumT.hpp>
#include <modelgbp/gbp/IntraGroupPolicyEnumT.hpp>
#include <modelgbp/gbp/UnknownFloodModeEnumT.hpp>
#include <modelgbp/gbp/BcastFloodModeEnumT.hpp>
#include <modelgbp/gbp/AddressResModeEnumT.hpp>
#include <modelgbp/gbp/RoutingModeEnumT.hpp>
#include <modelgbp/gbp/ConnTrackEnumT.hpp>

#include "logging.h"
#include "Endpoint.h"
#include "EndpointManager.h"
#include "EndpointListener.h"
#include "VppManager.h"
#include "Packets.h"
#include "VppOM.hpp"
#include "VppInterface.hpp"
#include "VppL2Config.hpp"
#include "VppL3Config.hpp"
#include "VppBridgeDomain.hpp"
#include "VppBoot.hpp"

#include "arp.h"
#include "eth.h"


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

    VppManager::VppManager(Agent& agent_,
                           IdGenerator& idGen_) :
        agent(agent_),
        taskQueue(agent.getAgentIOService()),
        idGen(idGen_),
        floodScope(FLOOD_DOMAIN),
        virtualRouterEnabled(true),
        routerAdv(false),
        virtualDHCPEnabled(false),
        stopping(false) {

        VPP::HW::init();
        VPP::OM::init();
        memset(routerMac, 0, sizeof(routerMac));
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
        VPP::HW::connect();

        /**
         * We are insterested in getting interface events from VPP
         */
        std::shared_ptr<VPP::Cmd> itf(new VPP::Interface::EventsCmd(*this));

        VPP::HW::enqueue(itf);
        m_cmds.push_back(itf);

        /**
         * We are insterested in getting DHCP events from VPP
         */
        std::shared_ptr<VPP::Cmd> dc(new VPP::DhcpConfig::EventsCmd(*this));

        VPP::HW::enqueue(dc);
        m_cmds.push_back(dc);
    }

    void VppManager::handleUplinkConfigure()
    {
        m_uplink.configure();
    }


    void VppManager::handleBoot() {
        std::string intfName{"tap0"};
        m_boot.dump();
        m_boot.createControlInterface(intfName);
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

    }

    void VppManager::setFloodScope(FloodScope fscope) {
        floodScope = fscope;
    }

    void VppManager::setVirtualRouter(bool virtualRouterEnabled,
                                      bool routerAdv,
                                      const string& virtualRouterMac) {
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

    void VppManager::handle_interface_event(VPP::Interface::EventsCmd *e)
    {
        if (stopping) return;
        taskQueue.dispatch("InterfaceEvent",
                           bind(&VppManager::handleInterfaceEvent,
                                this, e));
    }

    void VppManager::handle_dhcp_event(VPP::DhcpConfig::EventsCmd *e)
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
        VPP::OM::remove(uuid);
        return;
    }

    /*
     * This is an update to all the state related to this endpoint.
     * At the end of processing we want all the state realted to this endpint,
     * that we don't touch here, gone.
     */
    VPP::OM::mark(uuid);

    /* if (!vppApi.isConnected()) { */
    /*     LOG(ERROR) << "VppApi is not connected"; */
    /*     return; */
    /* } */

    const Endpoint& endPoint = *epWrapper.get();
    const optional<string>& vppInterfaceName = endPoint.getInterfaceName();
    int rv;

    /*
     * We want a veth interface - admin up
     */
    VPP::Interface itf(vppInterfaceName.get(),
                       VPP::Interface::type_t::AFPACKET,
                       VPP::Interface::admin_state_t::UP);
    VPP::OM::write(uuid, itf);

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
        // I don't believe we need it in VPP
        /* address_v6 linkLocalIp(packets::construct_link_local_ip_addr(macAddr));
        if (endPoint.getIPs().find(linkLocalIp.to_string()) ==
            endPoint.getIPs().end())
            ipAddresses.push_back(linkLocalIp);
        */
    }

    optional<URI> epgURI = epMgr.getComputedEPG(uuid);
    if (!epgURI) {      // can't do much without EPG
        return;
    }

    uint32_t epgVnid, rdId, bdId, fgrpId;
    optional<URI> fgrpURI, bdURI, rdURI;
    if (!getGroupForwardingInfo(epgURI.get(), epgVnid, rdURI,
                            rdId, bdURI, bdId, fgrpURI, fgrpId)) {
        return;
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

        VPP::BridgeDomain bd(fgrpId);

        if (VPP::rc_t::OK != VPP::OM::write(uuid, bd))
        {
            LOG(ERROR) << "VppApi did not create bridge: "
                       << *fd.get()->getName()
                       << " for port: "
                       << vppInterfaceName.get();
            return;
        }

        VPP::L2Config l2itf(itf, bd);

        if (VPP::rc_t::OK != VPP::OM::write(uuid, l2itf))
        {
            LOG(ERROR) << "VppApi did not set bridge: "
                       << *fd.get()->getName()
                       << " for port: "
                       << vppInterfaceName.get();
            return;
        }
    }

    /* if (rdId != 0 && bdId != 0 && */
    /*     vppApi.getBridgeNameByIntf(vppInterfaceName.get()).first) { */
    /*     uint8_t routingMode = */
    /*     agent.getPolicyManager().getEffectiveRoutingMode(epgURI.get()); */

    /*     if (virtualRouterEnabled && hasMac && */
    /*         routingMode == RoutingModeEnumT::CONST_ENABLED) { */
    /*         for (const address& ipAddr : ipAddresses) { */
    /*             if (endPoint.isDiscoveryProxyMode()) { */
    /*                 /\* Auto-reply to ARP and NDP requests for endpoint */
    /*                  * IP addresses */
    /*                  * TODO I believe that VPP handles it by default. */
    /*                  * May need to check with keith. */
    /*                  * if VPP will not handle it, we can think about */
    /*                  * "set ip arp" or "set ip6 nd proxy" */
    /*                  * flowsProxyDiscovery(*this, elBridgeDst, 20, ipAddr, */
    /*                  *\/  */
    /*             } else { */
    /*                 if (arpMode != AddressResModeEnumT::CONST_FLOOD && */
    /*                     ipAddr.is_v4()) { */
    /*                     if (arpMode == AddressResModeEnumT::CONST_UNICAST) { */
    /*                     /\* */
    /*                      * TODO ARP optimization: broadcast -> unicast */
    /*                      * vpp API: "set ip ARP" */
    /*                      *\/ */
    /*                     } else { */
    /*                     /\* */
    /*                      * Keith, how VPP will handle such arp requests which */
    /*                      * do not have entries (using "set ip arp") in the VPP. */
    /*                      *\/ */
    /*                      // drop the ARP packet */
    /*                     } */
    /*                 } */
    /*                 if (ndMode != AddressResModeEnumT::CONST_FLOOD && */
    /*                     ipAddr.is_v6()) { */
    /*                     if (ndMode == AddressResModeEnumT::CONST_UNICAST) { */
    /*                         /\* */
    /*                          * TODO neighbor discovery optimization: */
    /*                          * broadcast -> unicast */
    /*                          *\/ */
    /*                         // vpp API: "set ip6 neighbor" */
    /*                         // actionDestEpArp(e1, epgVnid, swIfIndex, macAddr); */
    /*                      } else { */
    /*                         // else drop the ND packet */
    /*                      } */
    /*                 } */
    /*             } */
    /*         } */

    /*         // IP address mappings */
    /*         for (const Endpoint::IPAddressMapping& ipm : */
    /*             endPoint.getIPAddressMappings()) { */
    /*             if (!ipm.getMappedIP() || !ipm.getEgURI()) */
    /*                 continue; */

    /*             address mappedIp = */
    /*                address::from_string(ipm.getMappedIP().get(), ec); */
    /*             if (ec) continue; */

    /*             address floatingIp; */
    /*             if (ipm.getFloatingIP()) { */
    /*                 floatingIp = */
    /*                     address::from_string(ipm.getFloatingIP().get(), ec); */
    /*                 if (ec) continue; */
    /*                 if (floatingIp.is_v4() != mappedIp.is_v4()) continue; */
    /*             } */

    /*             uint32_t fepgVnid, frdId, fbdId, ffdId; */
    /*             optional<URI> ffdURI, fbdURI, frdURI; */
    /*             if (!getGroupForwardingInfo(ipm.getEgURI().get(), fepgVnid, */
    /*                                         frdURI, frdId, fbdURI, fbdId, */
    /*                                         ffdURI, ffdId)) */
    /*                 continue; */

    /*             uint32_t nextHop; */
    /*             if (ipm.getNextHopIf()) { */
    /*             // nextHop = switchManager.getPortMapper() */
    /*             //    .FindPort(ipm.getNextHopIf().get()); */
    /*             // if (nextHop == VppApi::VPP_SWIFINDEX_NONE) continue; */
    /*             } */
    /*             uint8_t nextHopMac[6]; */
    /*             const uint8_t* nextHopMacp = NULL; */
    /*             if (ipm.getNextHopMAC()) { */
    /*                 ipm.getNextHopMAC().get().toUIntArray(nextHopMac); */
    /*                  nextHopMacp = nextHopMac; */
    /*             } */
    /*         /\* */
    /*          * Keith what is about IP address mapping? */
    /*          * Why do we need it? */
    /*          * How VPP does play a role there? */
    /*          *\/ */
    /*         } */
    /*     } */
    /* } */

    /* if (fgrpURI && vppApi.getIntfIndexByName(vppInterfaceName.get()).first) { */
    /*     updateEndpointFloodGroup(fgrpURI.get(), endPoint, */
    /*         vppApi.getIntfIndexByName(vppInterfaceName.get()).second, */
    /*                              endPoint.isPromiscuousMode(), */
    /*                              fd); */
    /* } else { */
    /*     removeEndpointFromFloodGroup(uuid); */
    /* } */

    /*
     * That's all folks ...
     */
    VPP::OM::sweep(uuid);
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
            VPP::OM::remove(epg_uuid);
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
        VPP::OM::mark(epg_uuid);

        /*
         * Construct the BridgeDomain
         */
        VPP::BridgeDomain bd(fgrpId);

        VPP::OM::write(epg_uuid, bd);

        /*
         * Construct the encap-link
         */
        std::shared_ptr<VPP::Interface> encap_link(m_uplink.mk_interface(epg_uuid, epgVnid));

        /*
         * Add the encap-link to the BD
         */
        VPP::L2Config l2(*encap_link, bd);
        VPP::OM::write(epg_uuid, l2);

        /*
         * Add the BVIs to the BD
         */
        optional<shared_ptr<RoutingDomain>> epgRd = pm.getRDForGroup(epgURI);

        VPP::RouteDomain rd(rdId);
        VPP::OM::write(epg_uuid, rd);

        updateBVIs(epgURI, bd, rd);

        /*
         * Sweep the remaining EPG's state
         */
        VPP::OM::sweep(epg_uuid);
    }

    void VppManager::updateBVIs(const URI& epgURI,
                                VPP::BridgeDomain &bd,
                                const VPP::RouteDomain &rd)
    {
        LOG(DEBUG) << "Updating BVIs";

        const string& epg_uuid = epgURI.toString();
        PolicyManager::subnet_vector_t subnets;
        agent.getPolicyManager().getSubnetsForGroup(epgURI, subnets);

        VPP::Interface bvi("bvi-" + std::to_string(bd.id()),
                           VPP::Interface::type_t::BVI,
                           VPP::Interface::admin_state_t::UP,
                           rd);
        VPP::OM::write(epg_uuid, bvi);

        VPP::L2Config l2(bvi, bd);
        VPP::OM::write(epg_uuid, l2);

        for (shared_ptr<Subnet>& sn : subnets)
        {
            optional<address> routerIp =
                PolicyManager::getRouterIpForSubnet(*sn);

            if (routerIp)
            {
                VPP::Route::prefix_t pfx(routerIp.get(), sn->getPrefixLen().get());

                VPP::L3Config l3(bvi, pfx);
                VPP::OM::write(epg_uuid, l3);
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
            return;
        }
        LOG(DEBUG) << "Updating routing domain " << rdURI;
    }

    void
    VppManager::handleDomainUpdate(class_id_t cid, const URI& domURI) {
        LOG(DEBUG) << "Updating domain " << domURI;
    }

    void
    VppManager::handleInterfaceEvent(VPP::Interface::EventsCmd *e)
    {
        LOG(DEBUG) << "Interface Event: " << *e;

        vapi_msg_sw_interface_set_flags event;

        while (e->pop(event))
        {
            VPP::handle_t handle(event.payload.sw_if_index);
            std::shared_ptr<VPP::Interface> sp = VPP::Interface::find(handle);

            if (sp)
            {
                VPP::Interface::oper_state_t oper_state =
                    VPP::Interface::oper_state_t::from_int(event.payload.link_up_down);

                LOG(INFO) << "Interface Event: " << sp->to_string()
                          << " state: " << oper_state.to_string();

                sp->set(oper_state);
            }
        }
    }
    
    void
    VppManager::handleDhcpEvent(VPP::DhcpConfig::EventsCmd *e)
    {
        LOG(INFO) << "Interface Event: " << *e;
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

    VPP::Boot &VppManager::boot()
    {
        return m_boot;
    }


} // namespace ovsagent
