#include "IPPacket.h"

IPPacket::IPPacket(ControlMessageId id) : Base (id),
  m_ip_header(nullptr),
  m_prevLayerPacket(nullptr, nullptr)
{
}

IPPacket::~IPPacket()
{
    m_ip_header = nullptr;
}

IPPacket::IPPacket(IPPacket&& orig) :
  m_ip_header(std::move(orig.m_ip_header)),
  m_prevLayerPacket(std::move(orig.m_prevLayerPacket))
{
    m_timeStamp = m_prevLayerPacket->m_timeStamp;
}

IPPacket::IPPacket(RawPacketsPoolItem&& orig) :
  m_prevLayerPacket(std::move(orig))
{
    m_ip_header = (HeaderType *) m_prevLayerPacket->getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket->m_timeStamp;
}

IPPacket &IPPacket::operator=(IPPacket &&orig)
{
    m_prevLayerPacket = std::move(orig.m_prevLayerPacket);
    m_ip_header = std::move(orig.m_ip_header);
    orig.m_ip_header = nullptr;

    m_timeStamp = m_prevLayerPacket->m_timeStamp;
    return *this;
}

const uint8_t *IPPacket::getDataImpl() const
{
    return m_prevLayerPacket->getPayloadDataImpl();
}

const uint8_t *IPPacket::getPayloadDataImpl() const
{
    return getDataImpl() + IPPacket::getHeaderSizeImpl();
}

size_t IPPacket::getHeaderSizeImpl() const
{
    return sizeof(HeaderType);
}

std::string inetIpToString(uint32_t ip)
{
    uint32_t iph = ntohl(ip);
    int i1 = (iph & 0xFF000000) >> 24;
    int i2 = (iph & 0x00FF0000) >> 16;
    int i3 = (iph & 0x0000FF00) >> 8;
    int i4 = (iph & 0x000000FF);
    return std::to_string(i1) + "." + std::to_string(i2) + "." + std::to_string(i3) + "." + std::to_string(i4);
}

std::string IPPacket::to_stringImpl() const
{
    std::string ret("EMPTY");
    if(m_prevLayerPacket)
    {
        ret = m_prevLayerPacket->to_stringImpl();
        ret = ret + ", ip total size: " + std::to_string(ntohs(m_ip_header->tot_len)) + ", src ip: " +
                    inetIpToString(m_ip_header->saddr) + ", dst ip: " +
                    inetIpToString(m_ip_header->daddr );
    }
    return ret;
}

size_t IPPacket::getPacketSpecificHashImpl() const
{
    size_t hash = 0;
    if(m_ip_header)
    {
        hash = m_ip_header->saddr + m_ip_header->daddr;
    }
    return hash;
}
