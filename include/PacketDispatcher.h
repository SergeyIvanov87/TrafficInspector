#ifndef PACKETDISPATCHER_H
#define PACKETDISPATCHER_H

#include <thread>
#include "CommonDataDefinition.h"
#include "Utils/Logger.h"
#include "PacketProcessor.h"
#include "Packets/RADIUSPacket.h"
#include <tuple>
#include <iostream>
#include <string.h>
#include <error.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <unistd.h>
#include "Utils/StaticHelpers.h"

template<class ...SupportedDispatchers>
class PacketDispatcher : public IDispatcher<ETHPacket, SupportedDispatchers...>
{
public:
    //Registered Dispatchers
    using Base = IDispatcher<ETHPacket, SupportedDispatchers...>;
    PacketDispatcher(SupportedDispatchers &&...dispatchers);
    ~PacketDispatcher();

    bool initialize(size_t sessionTimeoutSeconds);
    void deinitialize();

    //send SYN timeout command to packetProcessors
    void syncThread();
 private:
    //sync timeout notifiers
    int m_timerHandler;
    std::thread m_syncronizerThread;

    //stop work flag
    bool m_stopFlag = false;

};
#include "PacketDispatcher.hpp"
#endif /* PACKETROUTER_H */
