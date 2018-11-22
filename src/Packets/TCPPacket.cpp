#include "Packets/TCPPacket.h"
#include <arpa/inet.h>

TCPPacket::TCPPacket(ControlMessageId id) : Base (id),
    m_prevLayerPacket(id)
{
    tcp_header = nullptr;
}


TCPPacket::~TCPPacket()
{
    tcp_header = nullptr;
}

TCPPacket::TCPPacket(TCPPacket&& orig) :
  m_prevLayerPacket(std::move(orig.m_prevLayerPacket)),
  tcp_header(std::move(orig.tcp_header))
{
    orig.tcp_header = nullptr;
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

TCPPacket::TCPPacket(IPPacket&& orig) :
  m_prevLayerPacket(std::move(orig))
{
    tcp_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

TCPPacket::TCPPacket(RawPacketsPoolItem &&orig) :
  m_prevLayerPacket(std::move(orig))
{
    tcp_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

TCPPacket &TCPPacket::operator=(TCPPacket &&orig)
{
    m_prevLayerPacket = std::move(orig.m_prevLayerPacket);
    tcp_header = std::move(orig.tcp_header);
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
    orig.tcp_header = nullptr;
    return *this;
}

const uint8_t *TCPPacket::getDataImpl() const
{
    return m_prevLayerPacket.getPayloadDataImpl();
}

const uint8_t *TCPPacket::getPayloadDataImpl() const
{
    return getDataImpl() + TCPPacket::getHeaderSizeImpl();
}

size_t TCPPacket::getHeaderSizeImpl() const
{
    return sizeof(HeaderType);
}

std::string TCPPacket::to_stringImpl() const
{
    std::string ret = m_prevLayerPacket.to_stringImpl();
    if(tcp_header)
    {

        ret = ret + ", src_port: " + std::to_string(ntohs(tcp_header->th_sport)) +
                ", dst_port:" + std::to_string(ntohs(tcp_header->th_dport));
    }
    return ret;
}

size_t TCPPacket::getPacketSpecificHashImpl() const
{
    size_t hash = m_prevLayerPacket.getPacketSpecificHashImpl();
    if(tcp_header)
    {
        hash = hash + tcp_header->th_dport + tcp_header->th_sport;
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
