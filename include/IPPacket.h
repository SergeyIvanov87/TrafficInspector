#ifndef IPPACKET_H
#define IPPACKET_H
#include "CommonDataDefinition.h"
#include<netinet/ip.h>    //Provides declarations for ip header

class IPPacket : public BasePacket<PacketType::IP, IPPacket>
{
private:
    typedef BasePacket<PacketType::IP, IPPacket> Base;
    IPPacket(ControlMessageId id);
    IPPacket(IPPacket&& orig);
    IPPacket(RawPacketsPoolItem&& orig);
public:
    friend class UDPPacket;
    friend class BasePacket<PacketType::IP, IPPacket>;
    friend class TCPPacket;

    IPPacket &operator=(IPPacket &&src);

    IPPacket(const IPPacket& orig) = delete;
    bool operator=(const IPPacket &src) = delete;
    ~IPPacket();

    //Interface  impl
    const uint8_t *getDataImpl() const;
    const uint8_t *getPayloadDataImpl() const;
    size_t getHeaderSizeImpl() const;
    std::string to_stringImpl() const;
    typedef struct iphdr HeaderType;

    const HeaderType* const getHeadetPtr() const
    {
         return m_ip_header;
    }

    size_t getPacketSizeImpl() const
    {
        return m_prevLayerPacket->getPacketSizeImpl() - m_prevLayerPacket->getHeaderSizeImpl();
    }

    size_t getPacketSpecificHashImpl() const;
    template <class SourcePacket>
    static bool isConvertiblePacket(SourcePacket *src, uint8_t **next_level_header);
public:
    HeaderType *m_ip_header;
    RawPacketsPoolItem m_prevLayerPacket;
};

template <class SourcePacket>
bool IPPacket::isConvertiblePacket(SourcePacket *src, uint8_t **next_level_header)
{
    if(!src)
    {
        return false;
    }
    ETHPacket * etherPacket = static_cast<ETHPacket *>(src);
    if(ntohs(etherPacket->getHeadetPtr()->ether_type) == ETHERTYPE_IP)
    {
        *next_level_header = const_cast<uint8_t *>(etherPacket->getPayloadDataImpl());
        return true;
    }
    return false;
}
#endif /* IPPACKET_H */

