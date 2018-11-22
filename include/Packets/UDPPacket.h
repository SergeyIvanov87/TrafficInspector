#ifndef UDPPACKET_H
#define UDPPACKET_H
#include <vector>
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/udp.h>   //Provides declarations for udp header

#include <string>

#include "Packets/IPPacket.h"

class UDPPacket : public BasePacket<PacketType::UDP, UDPPacket>
{
private:
    typedef BasePacket<PacketType::UDP, UDPPacket> Base;
    UDPPacket(ControlMessageId id);
    UDPPacket(UDPPacket&& orig);
    UDPPacket(IPPacket &&orig);
    UDPPacket(RawPacketsPoolItem &&orig);

public:
    friend class BasePacket<PacketType::UDP, UDPPacket>;
    friend class RADIUSPacket;

    UDPPacket(const UDPPacket& orig) = delete;
    bool operator=(const UDPPacket &src) = delete;
    UDPPacket  &operator=(UDPPacket &&src);
     ~UDPPacket();

    //Interface
    const uint8_t *getDataImpl() const;
    const uint8_t *getPayloadDataImpl() const;
    size_t getHeaderSizeImpl() const;
    std::string to_stringImpl() const;

    typedef struct udphdr HeaderType;
    const HeaderType* const getHeadetPtr() const
    {
        return udp_header;
    }
    size_t getPacketSizeImpl() const
    {
        return m_prevLayerPacket.getPacketSizeImpl() - m_prevLayerPacket.getHeaderSizeImpl();
    }
    size_t getPacketSpecificHashImpl() const;

    //IDispatchable interface impl
    template <class SourcePacket>
    static bool isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header);
public:
    IPPacket m_prevLayerPacket;
    HeaderType *udp_header;
};

template <class SourcePacket>
bool UDPPacket::isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header)
{
    uint8_t *next_level_header_ip;
    if(IPPacket::isDispatchableTypeImpl(src, &next_level_header_ip))
    {
        IPPacket::HeaderType *ipHeader = (IPPacket::HeaderType *)next_level_header_ip;
        if(ipHeader->protocol == 17)    //TODO magic number
        {
            *next_level_header = next_level_header_ip + sizeof(IPPacket::HeaderType);
            return true;
        }
    }
    return false;
}
#endif /* UDPPACKET_H */
