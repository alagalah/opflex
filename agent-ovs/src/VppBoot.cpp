#include "VppDumpCmd.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppBoot.hpp"
#include "VppL3Config.hpp"
#include "VppL2Config.hpp"

using namespace VPP;

static const std::string BOOT_KEY = "__boot__";

Boot::Boot():
    completed(false)
{
}

void Boot::start()
{
    if (completed)
        return;

    dump_interface();
    dump_vxlan();
    dump_bridge();

    /*
     * having read all the state from VPP, mark it stale
     * we'll sweep once we get the converged signal
     */
    OM::mark(BOOT_KEY);

    completed = true;
}

void Boot::dump_interface()
{
    /*
     * dump VPP current states
     */
    Interface::DumpCmd::details_type *data;
    std::shared_ptr<Interface::DumpCmd> cmd(new Interface::DumpCmd());

    HW::enqueue(cmd);
    HW::write();


    while (data = cmd->pop())
    {
        std::unique_ptr<Interface> itf = Interface::new_interface(*data);

        LOG(ovsagent::DEBUG) << "dump: " << itf->to_string();

        if (itf && Interface::type_t::LOCAL != itf->type())
        {
            /*
             * Write each of the discovered interfaces into the OM,
             * but disable the HW Command q whilst we do, so that no
             * commands are sent to VPP
             */
            VPP::OM::commit(BOOT_KEY, *itf);

            /**
             * Get the address configured on the interface
             */
            L3Config::DumpV4Cmd::details_type *record;
            std::shared_ptr<L3Config::DumpV4Cmd> dcmd =
                std::make_shared<L3Config::DumpV4Cmd>(L3Config::DumpV4Cmd(itf->handle()));

            HW::enqueue(dcmd);
            HW::write();

            while (record = dcmd->pop())
            {
                const Route::prefix_t pfx(record->is_ipv6,
                                          record->ip,
                                          record->prefix_length);

                LOG(ovsagent::DEBUG) << "dump: " << pfx.to_string();

                L3Config l3(*itf, pfx);
                OM::commit(BOOT_KEY, l3);

                free(record);
            }
        }

        free(data);
    }
}

void Boot::dump_bridge()
{
    /*
     * dump VPP Bridge domains
     */
    BridgeDomain::DumpCmd::details_type *record;
    std::shared_ptr<BridgeDomain::DumpCmd> cmd(new BridgeDomain::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    while (record = cmd->pop())
    {
        BridgeDomain bd(record->bd_id);

        LOG(ovsagent::DEBUG) << "dump: " << bd.to_string();

        /*
         * Write each of the discovered interfaces into the OM,
         * but disable the HW Command q whilst we do, so that no
         * commands are sent to VPP
         */
        VPP::OM::commit(BOOT_KEY, bd);

        /**
         * For each interface in the BD construct an L2Config
         */
        for (int ii = 0; ii < record->n_sw_ifs; ii++)
        {
            std::shared_ptr<Interface> itf =
                Interface::find(record->sw_if_details[ii].sw_if_index);
            L2Config l2(*itf, bd);
            OM::commit(BOOT_KEY, l2);
        }

        free(record);
    }
}

/**
 * Initalse the connection to VPP
 */
void Boot::converged()
{
    /*
     * Sweep all the stale state
     */
    LOG(ovsagent::INFO) << "sweep";

    OM::sweep(BOOT_KEY);
}

void Boot::dump_vxlan()
{
    /*
     * dump VPP current states
     */
    VxlanTunnel::DumpCmd::details_type *data;
    std::shared_ptr<VxlanTunnel::DumpCmd> cmd(new VxlanTunnel::DumpCmd());

    HW::enqueue(cmd);
    HW::write();

    while (data = cmd->pop())
    {
        handle_t hdl(data->sw_if_index);
        boost::asio::ip::address src = from_bytes(data->is_ipv6,
                                                  data->src_address);
        boost::asio::ip::address dst = from_bytes(data->is_ipv6,
                                                  data->dst_address);

        VxlanTunnel vt(hdl, src, dst, data->vni);

        LOG(ovsagent::DEBUG) << "dump: " << vt.to_string();

        OM::commit(BOOT_KEY, vt);
    }
}
