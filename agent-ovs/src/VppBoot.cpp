#include "VppDumpCmd.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppBoot.hpp"

using namespace VPP;

static const std::string BOOT_KEY = "__boot__";

Boot::Boot():
    completed(false)
{

}

void Boot::dump()
{
    if (completed)
        return;

    std::shared_ptr<Interface::DumpInterfaceCmd> cmd(new Interface::DumpInterfaceCmd());

    HW::enqueue(cmd);
    HW::write();

    Interface::DumpInterfaceCmd::details_type data;

    while (cmd->pop(data))
    {
        Interface itf(&data);
       // VPP::OM::write(BOOT_KEY, itf);
        LOG(ovsagent::INFO) << "dump: " << itf.to_string();
    }


    completed = true;
}

