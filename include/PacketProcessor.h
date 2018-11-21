#ifndef PACKETPROCESSOR_H
#define PACKETPROCESSOR_H
#include "CommonDataDefinition.h"
#include "BasePacket.h"
#include "UDPPacket.h"
#include "TCPPacket.h"
#include "RADIUSPacket.h"
#include "ObjectQueue.h"
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

    //interface for push regular packets
    bool pushPacket(PacketProcessorQueueItem &&inputPacket);

    //interface for push control packets
    bool pushPacket(ControlMessageId inputPacket);
    PacketProcessorQueueItem popPacket();
    bool isStopped() const{ return m_stop; }
    constexpr const char* getProtocolName() const {return PacketType2String(SpecificPacket::getPacketType());}
    void setTimoutForWaitingResponse(size_t timeoutMsec) { m_responseTimeoutMsec = timeoutMsec; }

private:

    //convert base packet from control packets
    template <class ControlPacket>
    static PacketProcessorQueueItem createNativeCtrlPacket(ControlPacket &&inputPacket);

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
