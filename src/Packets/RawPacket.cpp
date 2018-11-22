#include <string>
#include "Packets/RawPacket.h"
#include <arpa/inet.h>

ETHPacket::ETHPacket()
{
    //preallocation
    m_data = (uint8_t *) calloc(MAX_PACKET_SIZE, sizeof(uint8_t));
    eth_header = (struct ether_header *) m_data;
    m_timeStamp = 0;
    m_size = 0;
}

ETHPacket::ETHPacket(ETHPacket&& orig)
{
    m_data = std::move(orig.m_data);
    eth_header = std::move(orig.eth_header);
    m_size = std::move(orig.m_size);
    m_timeStamp = std::move(orig.m_timeStamp);

    orig.m_data = nullptr;
    orig.eth_header = nullptr;
    orig.m_size = 0;
    orig.m_timeStamp = 0;
}

ETHPacket &ETHPacket::operator=(ETHPacket &&src)
{
    m_data = std::move(src.m_data);
    eth_header = std::move(src.eth_header);
    m_size = std::move(src.m_size);
    m_timeStamp = std::move(src.m_timeStamp);

    src.m_data = nullptr;
    src.eth_header = nullptr;
    src.m_size = 0;
    src.m_timeStamp = 0;

    return *this;
}

ETHPacket::~ETHPacket()
{
    eth_header = nullptr;
    free(m_data);
    m_size = 0;
}

const uint8_t *ETHPacket::getDataImpl() const
{
    return m_data;
}

const uint8_t *ETHPacket::getPayloadDataImpl() const
{
    return m_data + sizeof(HeaderType);
}

size_t ETHPacket::getHeaderSizeImpl() const
{
    return sizeof(HeaderType);
}

std::string ETHPacket::to_stringImpl() const
{
    std::string ret;
    ret = ret + "ether_type: " + std::to_string(eth_header->ether_type);
    return ret;
}

size_t ETHPacket::getPacketSpecificHashImpl() const
{
    size_t hash = 0;
    if(eth_header)
    {
        for(int i = 0; i < ETH_ALEN; i++)
        {
            hash = hash + eth_header->ether_shost[i] + eth_header->ether_dhost[i];
        }
        hash += eth_header->ether_type;
    }
    return hash;
}
