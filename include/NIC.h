#ifndef NIC_H
#define NIC_H

#include "CommonDataDefinition.h"

class NIC
{
public:
    NIC(size_t rawPacketPoolSize);
    NIC(const NIC& orig) = delete;
    ~NIC();

    bool initialize(const char *ifName);
    RawPacketsPoolItem receivePacket();
private:
    int m_sniffSocket;

    RawPacketsPool m_nicRecvPacketsPool;
};

#endif /* NIC_H */

