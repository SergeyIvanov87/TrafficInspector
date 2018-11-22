#ifndef TCPPACKET_H
#define TCPPACKET_H
#include <vector>
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/udp.h>   //Provides declarations for udp header

#include <string>

#include "Packets/IPPacket.h"

class TCPPacket : public BasePacket<PacketType::TCP, TCPPacket>
{
private:
    typedef BasePacket<PacketType::TCP, TCPPacket> Base;
    TCPPacket(ControlMessageId id);
    TCPPacket(TCPPacket&& orig);
    TCPPacket(IPPacket &&orig);
    TCPPacket(RawPacketsPoolItem &&orig);

public:
    friend class BasePacket<PacketType::TCP, TCPPacket>;
    friend class RADIUSPacket;
    //friend class BasePacket<PacketType::UDP, UDPPacket>;

    TCPPacket(const TCPPacket& orig) = delete;
    bool operator=(const TCPPacket &src) = delete;
    TCPPacket  &operator=(TCPPacket &&src);
     ~TCPPacket();

    //Interface
    const uint8_t *getDataImpl() const;
    const uint8_t *getPayloadDataImpl() const;
    size_t getHeaderSizeImpl() const;
    std::string to_stringImpl() const;

    typedef struct tcphdr HeaderType;
    const HeaderType* const getHeadetPtr() const
    {
        return tcp_header;
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
    HeaderType *tcp_header;
};

template <class SourcePacket>
bool TCPPacket::isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header)
{
    uint8_t *next_level_header_ip;
    if(IPPacket::isDispatchableType(src, &next_level_header_ip))
    {
        IPPacket::HeaderType *ipHeader = (IPPacket::HeaderType *)next_level_header_ip;
        if(ipHeader->protocol == 6)    //TODO magic number
        {
            *next_level_header = next_level_header_ip + sizeof(IPPacket::HeaderType);
            return true;
        }
    }
    return false;
}
#endif /* TCPPACKET_H */
