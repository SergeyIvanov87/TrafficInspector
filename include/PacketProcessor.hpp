#ifndef PACKETPROCESSOR_HPP
#define PACKETPROCESSOR_HPP
#include <unordered_map>
#include "Utils/ObjectQueue.h"
#include "Utils/Logger.h"
#include "PacketProcessor.h"
#include "ResultNotifier.h"

#include "Dispatcher/IDispatcher.hpp"
#include <algorithm>


template<class SpecificPacket>
PacketProcessor<SpecificPacket>::PacketProcessor(size_t instancesCount, size_t responseTimeoutMsec/* = 2000*/):
m_receivedPacketQueue(instancesCount),
m_stop(false)
{
    //created processing thread by 'instancesCount'
    for(size_t index = 0; index < instancesCount; index++)
    {
        m_processingThread.emplace_back(&PacketProcessor<SpecificPacket>::processPacketsThread,
                            this,
                           std::ref(m_receivedPacketQueue[index]), std::ref(m_stop));
    }
}

template<class SpecificPacket>
PacketProcessor<SpecificPacket>:: ~PacketProcessor()
{
    //TODO send finish event & join before
    std::for_each(m_processingThread.begin(), m_processingThread.end(), [](auto &thread){thread.join();});
}

//IDispatcher Interface
template<class SpecificPacket>
template<class Type>
std::optional<size_t> PacketProcessor<SpecificPacket>::canBeDispatcheredImpl(const Type &inst, uint8_t **outInfo)
{
    std::optional<size_t> ret;
    if(PacketProcessorPacket::isDispatchableType(inst, outInfo))
    {
        ret = 0;    //curr instance index (myself)
    }
    return ret;
}

template<class SpecificPacket>
template<class Type>
bool PacketProcessor<SpecificPacket>::onDispatchImpl(Type &&inst)
{
    PacketProcessorQueueItem specialPacket(SpecificPacket::createPacketPtr(std::move(inst)));

    size_t workerId = inst->getPacketSpecificHash() % m_receivedPacketQueue.size();
    m_receivedPacketQueue[workerId].putObject(std::move(specialPacket));
    return true;
}

template<class SpecificPacket>
template<class Type>
bool PacketProcessor<SpecificPacket>::onDispatchBroadcastImpl(const Type &inst)
{
    static_assert(std::is_same_v<Type, ControlMessageId>, "PacketProcessor<SpecificPacket>::onDispatchBroadcastImpl -- only ControlMessageId is supported");
    //TODO
    return pushPacket(inst);
}

template<class SpecificPacket>
constexpr std::string PacketProcessor<SpecificPacket>::to_stringImpl() const
{
    return getProtocolName();
}

//Convert rawpacket or command packet into specific pcket type
template<class SpecificPacket>
template <class ControlPacket>
typename PacketProcessor<SpecificPacket>::PacketProcessorQueueItem PacketProcessor<SpecificPacket>::createNativeCtrlPacket(ControlPacket &&inputPacket)
{
    //Convert control packet for spcefic packet
    PacketProcessorQueueItem ctrlPacket(new typename PacketProcessorQueueItem::pointer());
    ctrlPacket->template setCtrlMessageId(inputPacket);
    PacketProcessorQueueItem ctrlMessage(ctrlPacket);
    return ctrlMessage;
}

template<class SpecificPacket>
bool PacketProcessor<SpecificPacket>::pushPacket(PacketProcessorQueueItem &&inputPacket)
{
    //balance packet with specific hash by processing threads.
    //Specific hash & balancing algorithm should provide session integrity
    //for example, a pair of source IP:port <-> destination IP:port should be balanced on the one worker thread
    size_t workerId = inputPacket->getPacketSpecificHash() % m_receivedPacketQueue.size();
    m_receivedPacketQueue[workerId].putObject(std::move(inputPacket));
    return true;
}

template<class SpecificPacket>
bool PacketProcessor<SpecificPacket>::pushPacket(ControlMessageId inputPacket)
{
    //Notify all workers about command packet -> SYNC for example
    for(auto &workerQueue : m_receivedPacketQueue)
    {
        workerQueue.putObject(PacketProcessorQueueItem(SpecificPacket::createPacketPtr(inputPacket)));
    }

    return true;
}

template<class SpecificPacket>
typename PacketProcessor<SpecificPacket>::PacketProcessorQueueItem PacketProcessor<SpecificPacket>::popPacket()
{
    return m_receivedPacketQueue.getObject();
}

template<class SpecificPacket>
void PacketProcessor<SpecificPacket>::processPacketsThread(PacketProcessorQueue &recvQueue, bool &stopFlag)
{
   processPacketsThreadImpl(recvQueue, stopFlag);
}



//Specific packets processing implementations
template<>
inline void PacketProcessor<UDPPacket>::processPacketsThreadImpl(PacketProcessorQueue &recvQueue, bool &stopFlag)
{
    logger("PacketProcessor<%s>::%s start", PacketType2String(UDPPacket::getPacketType()), __FUNCTION__);
    typedef PacketProcessor<UDPPacket>::PacketProcessorQueueItem UDPacketFromQueue;
    std::unordered_map<uint32_t, UDPacketFromQueue> packetSessionsMap;
    while(!stopFlag)
    {
        UDPacketFromQueue &&packet = recvQueue.getObject();
        logger("PacketProcessor<%s>::%s  - receive packet: %s", PacketType2String(UDPPacket::getPacketType()), __FUNCTION__, packet->to_string().c_str());

        //Control message types processing
        if(packet->getCtrlMessageId() == ControlMessageId::SHUTDOWN)
        {
            logger("PacketProcessor<%s>::%s - SHUTDOWN is requested", PacketType2String(UDPPacket::getPacketType()), __FUNCTION__);

            m_stop = true;
            break;
        }

        if(packet->getCtrlMessageId() == ControlMessageId::SYNC_TIMEOUT)
        {
            logger("PacketProcessor<%s>::%s - SYNC_TIMEOUT is requested", PacketType2String(UDPPacket::getPacketType()), __FUNCTION__);
            continue;
        }
    }
    logger("PacketProcessor<%s>::%s - finished", PacketType2String(UDPPacket::getPacketType()), __FUNCTION__);
}

/////////////////////////////RADIUS////////////////////////////////////////////
template<>
inline void PacketProcessor<RADIUSPacket>::processPacketsThreadImpl(PacketProcessorQueue &recvQueue, bool &stopFlag)
{
    logger("PacketProcessor<%s>::%s start", PacketType2String(RADIUSPacket::getPacketType()), __FUNCTION__);
    typedef PacketProcessor<RADIUSPacket>::PacketProcessorQueueItem PacketFromQueue;

    std::unordered_map<rad_session_pair, PacketFromQueue> packetSessionsMap;
    size_t totalPacketsTimeoutCounter = 0;
    size_t intervalPacketsTimeoutCounter = 0;
    while(!stopFlag)
    {
        PacketFromQueue &&packet = recvQueue.getObject();

        //Control message types processing
        if(packet->getCtrlMessageId() == ControlMessageId::SHUTDOWN)
        {
            logger("PacketProcessor<%s>::%s  - SHUTDOWN requested", PacketType2String(RADIUSPacket::getPacketType()), __FUNCTION__);

            m_stop = true;
            break;
        }

        if(packet->getCtrlMessageId() == ControlMessageId::SYNC_TIMEOUT)
        {
            logger("PacketProcessor<%s>::%s  - SYNC_TIMEOUT is requested", PacketType2String(RADIUSPacket::getPacketType()), __FUNCTION__);

            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            size_t timeStamp = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
            for(auto iter = packetSessionsMap.begin(); iter != packetSessionsMap.end(); )
            {
                size_t delta = timeStamp - iter->second->getTimestamp();
                if(delta >= m_responseTimeoutMsec)
                {
                    intervalPacketsTimeoutCounter++;
                    totalPacketsTimeoutCounter ++;
                    iter = packetSessionsMap.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
            logger("PacketProcessor<%s>::%s  - SYNC finished: unresponded - total: %zu, interval: %zu",
                                        PacketType2String(RADIUSPacket::getPacketType()),
                                        __FUNCTION__, totalPacketsTimeoutCounter, intervalPacketsTimeoutCounter);

            //reset interval counter
            intervalPacketsTimeoutCounter = 0;
            continue;
        }
        logger("PacketProcessor<%s>::%s  - receive packet: %s", PacketType2String(RADIUSPacket::getPacketType()), __FUNCTION__, packet->to_string().c_str());

        //test on already created session+identifier
        rad_session_pair sp = packet->getPacketSessionPair();
        auto range = packetSessionsMap.equal_range(sp);
        bool match = false;
        auto it = range.first;
        for (; it != range.second; ++it)
        {
            if(it->first == sp)
            {
                match = true;
                ResultNotifier::instance().provideDataForNotifier(*packet, *(it->second));
                break;
            }
        }

        //remove matched pair
        if(match)
        {
            packetSessionsMap.erase(it);
        }
        else
        {
            packetSessionsMap.insert(std::make_pair(std::move(sp), std::move(packet)));
        }
    }
    logger("PacketProcessor<%s>::%s - finished", PacketType2String(RADIUSPacket::getPacketType()), __FUNCTION__);
}

#endif /* PACKETPROCESSOR_HPP */


//TCP
template<>
inline void PacketProcessor<TCPPacket>::processPacketsThreadImpl(PacketProcessorQueue &recvQueue, bool &stopFlag)
{
    logger("PacketProcessor<%s>::%s start", PacketType2String(TCPPacket::getPacketType()), __FUNCTION__);
    typedef PacketProcessor<TCPPacket>::PacketProcessorQueueItem TCPPacketFromQueue;
    std::unordered_map<uint32_t, TCPPacketFromQueue> packetSessionsMap;
    while(!stopFlag)
    {
        TCPPacketFromQueue &&packet = recvQueue.getObject();
        logger("PacketProcessor<%s>::%s  - receive packet: %s", PacketType2String(TCPPacket::getPacketType()), __FUNCTION__, packet->to_string().c_str());

        //Control message types processing
        if(packet->getCtrlMessageId() == ControlMessageId::SHUTDOWN)
        {
            logger("PacketProcessor<%s>::%s - SHUTDOWN is requested", PacketType2String(TCPPacket::getPacketType()), __FUNCTION__);

            m_stop = true;
            break;
        }

        if(packet->getCtrlMessageId() == ControlMessageId::SYNC_TIMEOUT)
        {
            logger("PacketProcessor<%s>::%s - SYNC_TIMEOUT is requested", PacketType2String(TCPPacket::getPacketType()), __FUNCTION__);
            continue;
        }
    }
    logger("PacketProcessor<%s>::%s - finished", PacketType2String(TCPPacket::getPacketType()), __FUNCTION__);
}
