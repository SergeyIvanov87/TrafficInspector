#include "UDPPacket.h"
#include <arpa/inet.h>

UDPPacket::UDPPacket(ControlMessageId id) : Base (id),
    m_prevLayerPacket(id)
{
    udp_header = nullptr;
}


UDPPacket::~UDPPacket()
{
    udp_header = nullptr;
}

UDPPacket::UDPPacket(UDPPacket&& orig) :
  m_prevLayerPacket(std::move(orig.m_prevLayerPacket)),
  udp_header(std::move(orig.udp_header))
{
    orig.udp_header = nullptr;
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

UDPPacket::UDPPacket(IPPacket&& orig) :
  m_prevLayerPacket(std::move(orig))
{
    udp_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

UDPPacket::UDPPacket(RawPacketsPoolItem &&orig) :
  m_prevLayerPacket(std::move(orig))
{
    udp_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

UDPPacket &UDPPacket::operator=(UDPPacket &&orig)
{
    m_prevLayerPacket = std::move(orig.m_prevLayerPacket);
    udp_header = std::move(orig.udp_header);
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
    orig.udp_header = nullptr;
    return *this;
}

const uint8_t *UDPPacket::getDataImpl() const
{
    return m_prevLayerPacket.getPayloadDataImpl();
}

const uint8_t *UDPPacket::getPayloadDataImpl() const
{
    return getDataImpl() + UDPPacket::getHeaderSizeImpl();
}

size_t UDPPacket::getHeaderSizeImpl() const
{
    return sizeof(HeaderType);
}

std::string UDPPacket::to_stringImpl() const
{
    std::string ret = m_prevLayerPacket.to_stringImpl();
    if(udp_header)
    {

        ret = ret + ", src_port: " + std::to_string(ntohs(udp_header->uh_sport)) +
                ", dst_port:" + std::to_string(ntohs(udp_header->uh_dport));
    }
    return ret;
}

size_t UDPPacket::getPacketSpecificHashImpl() const
{
    size_t hash = m_prevLayerPacket.getPacketSpecificHashImpl();
    if(udp_header)
    {
        hash = hash + udp_header->uh_dport + udp_header->uh_sport;
    }
    return hash;
}
/*
UDPPacket UDPPacket::createPacket(RawPacketsPoolItem &&src)
{
    return UDPPacket(std::move(src));
}
UDPPacket *UDPPacket::createPacketPtr(RawPacketsPoolItem &&src)
{
    return new UDPPacket(std::move(src));
}*/
