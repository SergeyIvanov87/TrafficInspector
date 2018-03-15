#include "RADIUSPacket.h"
#include <algorithm>
#include <string>

namespace std
{
    string to_string(const AVPStorage &storage)
    {
        string ret;
        for( const auto &avp : storage)
        {
            string vec(avp.second.size(), 0);
            std::transform(avp.second.begin(), avp.second.end(), vec.begin(), [](uint8_t value)
            {
                return static_cast<char>(value == 0 ? '0' : value);
            });
            ret = ret + "[Type: " + to_string((int)avp.first)
                      + ", Length: " + to_string(avp.second.size())
                      + ", Value: " + vec + "], ";
        }
        return ret;
    }
}

RADIUSPacket::RADIUSPacket(ControlMessageId id) : Base (id),
  m_prevLayerPacket(id),
  radius_header(nullptr)
{
}

RADIUSPacket::~RADIUSPacket()
{
    radius_header = nullptr;
}

RADIUSPacket::RADIUSPacket(RADIUSPacket&& orig) :
  m_prevLayerPacket(std::move(orig.m_prevLayerPacket)),
  radius_header(std::move(orig.radius_header))
{
    orig.radius_header = nullptr;
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
}

RADIUSPacket::RADIUSPacket(UDPPacket&& orig) :
  m_prevLayerPacket(std::move(orig))
{
    radius_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();

    m_timeStamp = m_prevLayerPacket.m_timeStamp;
    uint16_t pack_lenght = ntohs(radius_header->m_length);
    assert(pack_lenght == getPacketSizeImpl());
    uint16_t size = std::min(getPacketSizeImpl(), (size_t)pack_lenght);
    uint16_t dataOffset = getHeaderSizeImpl();
    const uint8_t *payload = getPayloadDataImpl();

    bool atrParsed = false, lenParsed = false, valParsed = false;
    while(size > dataOffset)
    {
        atrParsed = false;
        radius_avp_type type;
        if((size > dataOffset + sizeof(type)))
        {
            type = *(static_cast<const radius_avp_type *>(payload));
            dataOffset += sizeof(type);
            payload += sizeof(type);

            atrParsed = true;
        }

        uint8_t length = 0;
        lenParsed = false;
        if((size > dataOffset + sizeof(length)) && atrParsed)
        {
            length = *(static_cast<const decltype(length) *>(payload));
            dataOffset += sizeof(length);
            payload += sizeof(length);

            lenParsed = true;
        }

        radius_avp_value value;
        valParsed = false;
        if((size > dataOffset +  length - 2) && lenParsed)
        {
            length -= 2;//minus type & length
            value.resize(length);
            memcpy(value.data(), payload, length);
            dataOffset += length;
            payload += length;

            valParsed = true;
        }

        if(valParsed)
        {
            m_avps.insert(std::make_pair(type, std::move(value)));
            valParsed = false;
        }
    }
}

RADIUSPacket::RADIUSPacket(RawPacketsPoolItem &&orig) :
  m_prevLayerPacket(std::move(orig))
{
    radius_header = (HeaderType *)m_prevLayerPacket.getPayloadDataImpl();
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
    uint16_t pack_lenght = ntohs(radius_header->m_length);
    assert(pack_lenght == getPacketSizeImpl());
    uint16_t size = std::min(getPacketSizeImpl(), (size_t)pack_lenght);
    uint16_t dataOffset = getHeaderSizeImpl();
    const uint8_t *payload = getPayloadDataImpl();

    bool atrParsed = false, lenParsed = false, valParsed = false;
    while(size > dataOffset)
    {
        atrParsed = false;
        radius_avp_type type;
        if((size > dataOffset + sizeof(type)))
        {
            type = *(static_cast<const radius_avp_type *>(payload));
            dataOffset += sizeof(type);
            payload += sizeof(type);

            atrParsed = true;
        }

        uint8_t length = 0;
        lenParsed = false;
        if((size > dataOffset + sizeof(length)) && atrParsed)
        {
            length = *(static_cast<const decltype(length) *>(payload));
            dataOffset += sizeof(length);
            payload += sizeof(length);

            lenParsed = true;
        }

        radius_avp_value value;
        valParsed = false;
        if((size >= dataOffset + length - sizeof(length) - sizeof(type)) && lenParsed)
        {
            length -= 2;//minus type & length
            value.resize(length);
            memcpy(value.data(), payload, length);
            dataOffset += length;
            payload += length;

            valParsed = true;
        }

        if(valParsed)
        {
            m_avps.insert(std::make_pair(type, std::move(value)));
            valParsed = false;
        }
        else
        {
            assert(false);
            break;
        }
    }
}

RADIUSPacket &RADIUSPacket::operator=(RADIUSPacket &&orig)
{
    m_prevLayerPacket = std::move(orig.m_prevLayerPacket);
    radius_header = std::move(orig.radius_header);
    m_timeStamp = m_prevLayerPacket.m_timeStamp;
    orig.radius_header = nullptr;

    return *this;
}

rad_session_pair RADIUSPacket::getPacketSessionPair() const
{
    rad_session_pair ret;
    if(getHeadetPtr())
    {
        ret.identifier = getHeadetPtr()->m_packetIdentifier;
    }
    return ret;
}

const uint8_t *RADIUSPacket::getDataImpl() const
{
    return m_prevLayerPacket.getPayloadDataImpl();
}

const uint8_t *RADIUSPacket::getPayloadDataImpl() const
{
    return getDataImpl() + RADIUSPacket::getHeaderSizeImpl();
}

size_t RADIUSPacket::getHeaderSizeImpl() const
{
    return sizeof(HeaderType);
}

std::string RADIUSPacket::to_stringImpl() const
{
    std::string ret = m_prevLayerPacket.to_stringImpl();
    if(radius_header)
    {
        ret = ret + ", code: " + std::to_string((int)radius_header->m_code) +
                ", packetIdentifier: " + std::to_string((int)radius_header->m_packetIdentifier) +
                ", length: " +  std::to_string(ntohs(radius_header->m_length));
                //+
                //"authentificator: " +  std::to_string(radius_header->m_authentificator);
    }
    return ret;
}

size_t RADIUSPacket::getPacketSpecificHashImpl() const
{
    size_t hash = m_prevLayerPacket.getPacketSpecificHashImpl();
    if(radius_header)
    {
        hash = hash + radius_header->m_packetIdentifier;
    }
    return hash;
}
