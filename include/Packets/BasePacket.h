#ifndef BASEPACKET_H
#define BASEPACKET_H
#include <assert.h>
#include <array>
#include <time.h>
#include "Dispatcher/IDispatchable.h"

enum class PacketType
{
    ETH,
    IP,
    UDP,
    TCP,
    RADIUS_UDP,

    //Auxilary types
    CONTROL_EVENT,
    MAX_PACKET_TYPE
};

inline constexpr const char *PacketType2String(PacketType t)
{
    const std::array<const char*, static_cast<int>(PacketType::MAX_PACKET_TYPE)> strings{
                                "ETH",
                                "IP",
                                "UDP",
                                "TCP",
                                "RADIUS_UDP",
                                "CONTROL_EVENT",
                                };
    static_assert(strings.size() == static_cast<int>(PacketType::MAX_PACKET_TYPE));
    return t < PacketType::MAX_PACKET_TYPE ? strings[static_cast<int>(t)] : "UNKNOWN";
}

enum class ControlMessageId
{
    PROCESSING,
    SYNC_TIMEOUT,
    SHUTDOWN,
    MAX_CONTROL_MESSAGE_ID
};

inline constexpr const char *ControlMessageId2String(ControlMessageId t)
{
    const std::array<const char*, static_cast<int>(ControlMessageId::MAX_CONTROL_MESSAGE_ID)> strings{
                                "PROCESSING",
                                "SYNC_TIMEOUT",
                                "SHUTDOWN"
                                };
    static_assert(strings.size() == static_cast<int>(ControlMessageId::MAX_CONTROL_MESSAGE_ID));
    return t < ControlMessageId::MAX_CONTROL_MESSAGE_ID ? strings[static_cast<int>(t)] : "UNKNOWN";
}


//Base class for packets, use CRTP
//Use preallocated data buffer MAX_PACKET_SIZE, to avoid
//unnescessary copy-data and allocation
template<PacketType type, class ConcretePacket>
struct BasePacket : public IDispatchable<
                                                    BasePacket<
                                                            type,
                                                            ConcretePacket
                                                            >
                                                 >
{

    //TODO do not allocate buffer for  NOT of 'ETH' packet types
    //because they use ETH as member
    enum
    {
        MAX_PACKET_SIZE = 4096
    };

    BasePacket() : ctrlMessageId(ControlMessageId::PROCESSING)
    {
    }
    BasePacket(ControlMessageId id) : ctrlMessageId(id)
    {
    }
    BasePacket(BasePacket &&src) = default;
    BasePacket &operator=(BasePacket &&src) = default;

    static constexpr size_t getMaxPacketSize()  { return  MAX_PACKET_SIZE; }
    static constexpr PacketType getPacketType() { return type; }

    ControlMessageId getCtrlMessageId() const { return ctrlMessageId; };
    void setCtrlMessageId(ControlMessageId id) {  ctrlMessageId = id; };

    //Static interface declaration to be implement in derived classes
    const uint8_t *getData() const
    {
        return static_cast<const ConcretePacket *>(this)->getDataImpl();
    }
    const uint8_t *getPayloadData() const
    {
        return static_cast<const ConcretePacket *>(this)->getPayloadDataImpl();
    }
    size_t getHeaderSize() const
    {
        return static_cast<const ConcretePacket *>(this)->getHeaderSizeImpl();
    }
    size_t getPacketSize() const
    {
        return static_cast<const ConcretePacket *>(this)->getPacketSizeImpl();
    }
    std::string to_string() const
    {
        std::string ret(PacketType2String(getPacketType()));
        return ret + ",  ControlMessageId: " + ControlMessageId2String(ctrlMessageId) + ", " + static_cast<const ConcretePacket *>(this)->to_stringImpl();
    }
    size_t getPacketSpecificHash() const
    {
        return static_cast<const ConcretePacket *>(this)->getPacketSpecificHashImpl();
    }

    size_t getTimestamp() const
    {
        return m_timeStamp;
    }

    void setTimestamp()
    {
        //TODO
        //simple timestamp, do not check overflow
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        m_timeStamp = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    //Fabric creation packet methods
    template<class T>
    static ConcretePacket createPacket(T &&src)
    {
        return ConcretePacket(std::forward<T>(src));
    }
    template<class T>
    static ConcretePacket *createPacketPtr(T &&src)
    {
        return new ConcretePacket(std::forward<T>(src));
    }

    template <class SourcePacket>
    static bool isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header)
    {
        return ConcretePacket::isDispatchableTypeImpl(src, next_level_header);
    }

    static constexpr const char *getTypeDescriptionImpl()
    {
        return PacketType2String(ConcretePacket::getPacketType());
    }
protected:
    ControlMessageId ctrlMessageId = ControlMessageId::PROCESSING;
    size_t m_timeStamp;
};
#endif /* BASEPACKET_H */
