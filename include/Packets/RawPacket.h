
#ifndef RAWPACKET_H
#define RAWPACKET_H
#include <netinet/ether.h>
#include "Packets/BasePacket.h"

struct ETHPacket : public BasePacket<PacketType::ETH, ETHPacket>
{
    friend class IPPacket;
    //friend class TCPPacket;
    friend class UDPPacket;
    ETHPacket();
    ETHPacket(ETHPacket&& orig);
    ETHPacket &operator=(ETHPacket &&src);

    ETHPacket(const ETHPacket& orig) = delete;
    bool operator=(const ETHPacket &src) = delete;
    ~ETHPacket();


     //Static interface definition
     typedef struct ether_header HeaderType;

     const uint8_t *getDataImpl() const;
     const uint8_t *getPayloadDataImpl() const;
     size_t getHeaderSizeImpl() const;
     std::string to_stringImpl() const;

     const HeaderType* const getHeadetPtr() const
     {
         return eth_header;
     }

     void setSize(size_t packetSize)
     {
         m_size = packetSize;
     }

     size_t getPacketSizeImpl() const
     {
         return m_size;
     }

     size_t getPacketSpecificHashImpl() const;
public:
    uint8_t *m_data;
    HeaderType *eth_header;
    size_t m_size;
};
#endif /* RAWPACKET_H */
