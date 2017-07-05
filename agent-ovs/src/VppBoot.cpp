#include "VppDumpCmd.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppBoot.hpp"
#include "VppL3Config.hpp"

using namespace VPP;

static const std::string BOOT_KEY = "__boot__";

Boot::Boot():
    completed(false) {

}

void Boot::dump() {
    if (completed)
        return;

    /*
     * dump VPP current states
     */
    {
        std::shared_ptr<L3Config::DumpV4Cmd> cmd(new L3Config::DumpV4Cmd());

        HW::enqueue(cmd);
        HW::write();

        L3Config::DumpV4Cmd::details_type data;

        while (cmd->pop(data))
        {
            Route::prefix_t pfx(0, data.address, data.address_length);

            LOG(ovsagent::INFO) << "dump: " << pfx.to_string();
        }
    }

    {
        std::shared_ptr<Interface::DumpInterfaceCmd> cmd(new Interface::DumpInterfaceCmd());

        HW::enqueue(cmd);
        HW::write();

        Interface::DumpInterfaceCmd::details_type data;

        while (cmd->pop(data)) {
            Interface itf(&data);
//          VPP::OM::write(BOOT_KEY, itf);
            LOG(ovsagent::INFO) << "dump: " << itf.to_string();
        }
    }

    completed = true;
}

void Boot::createControlInterface(std::string &interfaceName) {

    Interface itf(interfaceName,
                  Interface::type_t::TAP,
                  Interface::admin_state_t::UP);

    VPP::OM::write(BOOT_KEY, itf);

    controlInterface = Interface::find(itf);
}
