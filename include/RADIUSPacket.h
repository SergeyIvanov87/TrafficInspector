
#ifndef RADIUSPACKET_H
#define RADIUSPACKET_H
#include "UDPPacket.h"
#include <array>
#include <limits>
#include <map>
#include <string.h>
#include <numeric>

struct radiusheader
{
    uint8_t m_code;
    uint8_t m_packetIdentifier;
    uint16_t m_length;
    uint8_t m_authentificator[16];
};

enum class RadiusAvpType
{
    AcctSessionId = 44
};


struct rad_session_pair
{
//    std::vector<uint8_t> session_id;
//    use identifier only
    uint8_t identifier;
};
inline bool operator== (const rad_session_pair &lhs, const rad_session_pair &rhs)
{
    return lhs.identifier == rhs.identifier;
    //&& lhs.session_id == rhs.session_id;
}

#include <functional>
namespace std
{
template<>
struct hash<rad_session_pair>
{
    std::size_t operator() ( const rad_session_pair& seq ) const
    {
        //just sum
        size_t sum = 0;
                  // std::accumulate(seq.session_id.begin(), seq.session_id.end(), 0);
        sum += seq.identifier;
        return sum;
    }
};
}

typedef uint8_t radius_avp_type;
typedef std::vector<uint8_t> radius_avp_value;
typedef std::map<radius_avp_type, radius_avp_value> AVPStorage;

namespace std
{
    string to_string(const AVPStorage &storage);
}

class RADIUSPacket : public BasePacket<PacketType::RADIUS_UDP, RADIUSPacket>
{
private:
    typedef BasePacket<PacketType::RADIUS_UDP, RADIUSPacket> Base;
    RADIUSPacket(ControlMessageId id);

    RADIUSPacket(RADIUSPacket&& orig);
    RADIUSPacket(UDPPacket &&orig);
    RADIUSPacket(RawPacketsPoolItem &&orig);

public:
    friend class BasePacket<PacketType::RADIUS_UDP, RADIUSPacket>;
    RADIUSPacket &operator=(RADIUSPacket &&src);
     ~RADIUSPacket();

    RADIUSPacket(const RADIUSPacket& orig) = delete;
    bool operator=(const RADIUSPacket &src) = delete;

    //Interface
    const uint8_t *getDataImpl() const;
    const uint8_t *getPayloadDataImpl() const;
    size_t getHeaderSizeImpl() const;
    std::string to_stringImpl() const;

    typedef radiusheader HeaderType;
    const HeaderType* const getHeadetPtr() const
    {
        return radius_header;
    }
    size_t getPacketSizeImpl() const
    {
        return m_prevLayerPacket.getPacketSizeImpl() - m_prevLayerPacket.getHeaderSizeImpl();
    }
    size_t getPacketSpecificHashImpl() const;
    rad_session_pair getPacketSessionPair() const;

    AVPStorage &getAVPs()
    {
        return m_avps;
    }
    const AVPStorage &getAVPs() const
    {
        return m_avps;
    }

    //IDispatchable interface impl
    template <class SourcePacket>
    static bool isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header);
protected:
    UDPPacket m_prevLayerPacket;
    HeaderType *radius_header;
    AVPStorage m_avps;
};

template <class SourcePacket>
bool RADIUSPacket::isDispatchableTypeImpl(const SourcePacket &src, uint8_t **next_level_header)
{
    uint8_t *next_level_header_ip;
    if(UDPPacket::isDispatchableTypeImpl(src, &next_level_header_ip))
    {
        UDPPacket::HeaderType *udpHeader = (UDPPacket::HeaderType *)next_level_header_ip;
        uint16_t dport = ntohs(udpHeader->uh_dport);
        uint16_t sport = ntohs(udpHeader->uh_sport);

        if((dport == 1812 || dport == 1813) ||
            (sport == 1812 || sport == 1813))
        {
            *next_level_header = next_level_header_ip + sizeof(IPPacket::HeaderType);
            return true;
        }
    }
    return false;
}

#endif /* RADIUSPACKET_H */
