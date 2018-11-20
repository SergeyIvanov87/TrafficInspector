#ifndef PACKETDISPATCHER_H
#define PACKETDISPATCHER_H

#include <thread>
#include "CommonDataDefinition.h"
#include "Logger.h"
#include "PacketProcessor.h"
#include "RADIUSPacket.h"
#include <tuple>
#include <iostream>
#include <string.h>
#include <error.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <unistd.h>

#include "StaticHelpers.h"


template<class ...SupportedDispatchers>
class PacketDispatcher
{
public:
    //Registered Dispatchers
    typedef std::tuple<SupportedDispatchers...> DispatchersPools;

    PacketDispatcher(SupportedDispatchers &&...dispatchers) :
        m_currentDispatchers(std::forward<SupportedDispatchers>(dispatchers)...),
        m_timerHandler(-1)
    {
    }
    ~PacketDispatcher();


    bool initialize(size_t sessionTimeoutSeconds);
    void deinitialize();
    
    //facade impl -> balance packet on specific worker
    template <class Packet>
    size_t route(Packet &&packet);
    size_t route(ControlMessageId packet);

    //send SYN timeout command to packetProcessors
    void syncThread();

    //just for info
    std::string getRegisteredProtocolNames() const;
 private:
     //current worker instances for protocol processing
    DispatchersPools m_currentDispatchers;

    //sync timeout notifiers
    int m_timerHandler;
    std::thread m_syncronizerThread;

    //stop work flag
    bool m_stopFlag = false;

};
#include "PacketDispatcherr.hpp"
#endif /* PACKETROUTER_H */
