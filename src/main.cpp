#include <cstdlib>
#include <array>
#include <vector>
#include <iostream>
#include "CommonObjectPool.h"
#include "NIC.h"
#include <string.h>
#include "PacketRouter.h"
#include "Logger.h"
#include <unistd.h>

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cout << "help --nic_name --raw_packet_pool_size" << std::endl;
        return -1;
    }

    //Get args   --- interface name, -- raw packet pool size
    std::string nicName(argv[1]);
    size_t rawPacketPoolSize(atoll(argv[2]));

    //logger 'facility'
    initializeLogger(nullptr);

    //create NIC
    NIC nic(rawPacketPoolSize);
    logger("Initialize NIC: %s with raw packets pool size: %zu", nicName.c_str(), rawPacketPoolSize);
    if(!nic.initialize(nicName.c_str()))
    {
        std::cerr << "Cannot initialize network interface card" << std::endl;
        exit(-1);
    }

    //Create data Notifier
    char hostName[32];
    gethostname(hostName, 32);
    ResultNotifier::instance().initialize(hostName);
    size_t sessionTimeout = 2;  //2 sec

    //Register packetsProcessor for PacketRouter...
    //You can use - multiple protocol here
    PacketRouter<
                RADIUSPacket,
                UDPPacket
                /*Your packet type HERE*/> packetRouter(1,1);

    logger("Initialize packetRouter:  timeout %zu sec", sessionTimeout);
    packetRouter.initialize(sessionTimeout);
    logger("PacketRouter<%s> initialized", packetRouter.getRegisteredProtocolNames().c_str());

    //Main loop
    while(true)
    {
        packetRouter.route(nic.receivePacket());
    }
    return 0;
}

