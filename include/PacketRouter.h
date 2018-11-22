#ifndef PACKETROUTER_H
#define PACKETROUTER_H

#include <thread>
#include "CommonDataDefinition.h"
#include "Utils/Logger.h"
#include "PacketProcessor.h"
#include <tuple>
#include <iostream>
#include <string.h>
#include <error.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <unistd.h>

#include "Utils/StaticHelpers.h"


template<class T, class Initializer>
struct InitializerSequenceWrapper
{
    InitializerSequenceWrapper(Initializer i) : init(i) {}
    Initializer init;
    typedef Initializer InitType;
};


//Facade for specific PacketProcessors<>
template<class ...SupportPacketsType>
class PacketRouter
{
public:
    //Registered PacketProcessor for specific packet types
    typedef std::tuple<SupportPacketsType...> SupportPacketsTypeTuple;
    typedef std::tuple<PacketProcessor<SupportPacketsType>...> PacketProcessorsPools;

    PacketRouter(InitializerSequenceWrapper<SupportPacketsType, size_t> ...initializers) :
        m_currentProcessors(initializers.init...),
        m_timerHandler(-1)
    {
    }
    ~PacketRouter();


    bool initialize(size_t sessionTimeoutSeconds);
    void deinitialize();

    //Convert/decode RAW packet for secific packet type
    template <class Packet, class IncominPacket>
    typename PacketProcessor<Packet>::PacketProcessorQueueItem createSpecificPacketPtr(IncominPacket &&incPacket);

    template <class Packet>
    PacketProcessor<Packet> &getSpecificPacketProcessor();

    //facade impl -> balance packet on specific worker
    template <class Packet>
    void route(Packet &&packet);
    void route(ControlMessageId packet);

    //send SYN timeout command to packetProcessors
    void syncThread();

    //just for info
    std::string getRegisteredProtocolNames() const;
 private:
     //current worker instances for protocol processing
    PacketProcessorsPools m_currentProcessors;

    //sync timeout notifiers
    int m_timerHandler;
    std::thread m_syncronizerThread;

    //stop work flag
    bool m_stopFlag = false;

};
#include "PacketRouter.hpp"
#endif /* PACKETROUTER_H */
