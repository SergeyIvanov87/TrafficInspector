#ifndef PACKETPROCESSOR_H
#define PACKETPROCESSOR_H
#include "CommonDataDefinition.h"
#include "Packets/BasePacket.h"
#include "Packets/UDPPacket.h"
#include "Packets/TCPPacket.h"
#include "Packets/RADIUSPacket.h"
#include "Utils/ObjectQueue.h"
#include "ResultNotifier.h"

#include "Dispatcher/IDispatcher.h"
#include <numeric>

//Use common logic for processing packet of different types
template<class SpecificPacket>
class PacketProcessor : public IDispatcher<SpecificPacket, PacketProcessor<SpecificPacket>>
{
public:
    using PacketProcessorPacket = SpecificPacket;
    using PacketProcessorQueueItem = std::unique_ptr<SpecificPacket>;

    //IDispatcher interface
    using ProcessingType = PacketProcessorPacket;
    template<class Type>
    std::optional<size_t> canBeDispatcheredImpl(const Type &inst, uint8_t **outInfo);

    template<class Type>
    bool onDispatchImpl(Type &&inst);

    template<class Type>
    bool onDispatchBroadcastImpl(const Type &inst);
    constexpr std::string to_stringImpl() const;
    
    //Type for inner queue for receive packets
    using PacketProcessorQueue = ObjectQueue<PacketProcessorQueueItem, WaitLockPolicy/*Test SpinLockPolicy*/>;

    PacketProcessor(size_t instancesCount, size_t responseTimeoutMsec = 2000);
    PacketProcessor(PacketProcessor &&src) = default;
    PacketProcessor &operator= (PacketProcessor &&src) = default;
    PacketProcessor(const PacketProcessor& orig) = delete;
    ~PacketProcessor();

    PacketProcessorQueueItem popPacket();
    bool isStopped() const{ return m_stop; }
    void setTimoutForWaitingResponse(size_t timeoutMsec) { m_responseTimeoutMsec = timeoutMsec; }

private:
    //thread for packet processing
    void processPacketsThread(PacketProcessorQueue &recvQueue, bool &stopFlag);
    size_t m_responseTimeoutMsec = 1000;
    std::vector<PacketProcessorQueue> m_receivedPacketQueue;

    std::vector<std::thread> m_processingThread;
    bool m_stop;

    //Override this method for specific packet types
    void processPacketsThreadImpl(PacketProcessorQueue &recvQueue, bool &stopFlag);
};

#include "PacketProcessor.hpp"
#endif /* PACKETPROCESSOR_H */
