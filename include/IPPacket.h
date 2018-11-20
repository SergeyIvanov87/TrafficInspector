#ifndef IPPACKET_H
#define IPPACKET_H
#include "CommonDataDefinition.h"
#include <type_traits> 
#include <netinet/ip.h>    //Provides declarations for ip header

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

    //Interfaces impl
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

    //IDispatchable interface impl
    template <class SourcePacket>
    static bool isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header);
public:
    HeaderType *m_ip_header;
    RawPacketsPoolItem m_prevLayerPacket;
};

template <class SourcePacket>
bool IPPacket::isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header)
{
    static_assert(std::is_base_of_v<ETHPacket, SourcePacket>, "IPPacket::isDispatchableTypeImpl  -- SourcePacket is not derived from ETHPacket");
    const ETHPacket &etherPacket = static_cast<const ETHPacket &>(src);
    if(ntohs(etherPacket.getHeadetPtr()->ether_type) == ETHERTYPE_IP)
    {
        *next_level_header = const_cast<uint8_t *>(etherPacket.getPayloadDataImpl());
        return true;
    }
    return false;
}
#endif /* IPPACKET_H */
