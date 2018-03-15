#include <unistd.h>
#include "NIC.h"
#include <iostream>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>

#include <sys/epoll.h>
#include <error.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>


#define ETHER_TYPE 0x0800
NIC::NIC(size_t rawPacketPoolSize) :
m_sniffSocket(-1),
  m_nicRecvPacketsPool(rawPacketPoolSize)
{
}

NIC::~NIC()
{
    close(m_sniffSocket);
}

bool NIC::initialize(const char *ifName)
{
    if( m_sniffSocket != -1)
    {
        return true;
    }

    m_sniffSocket = socket(PF_PACKET, SOCK_RAW, /*IPPROTO_UDP*/htons(ETHERTYPE_IP));
    if(m_sniffSocket == -1)
    {
        perror(strerror(errno));
        return false;
    }

    /* Set interface to promiscuous mode */
    struct ifreq ifopts;    /* set promiscuous mode */
//  struct ifreq if_ip; /* get ip addr */
//  struct sockaddr_storage their_addr;
    int sockopt;
    strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
    ioctl(m_sniffSocket, SIOCGIFFLAGS, &ifopts);
    ifopts.ifr_flags |= IFF_PROMISC;
    ioctl(m_sniffSocket, SIOCSIFFLAGS, &ifopts);
    /* Allow the socket to be reused - incase connection is closed prematurely */
    if (setsockopt(m_sniffSocket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1)
        {
        perror("setsockopt");
        close(m_sniffSocket);
        exit(EXIT_FAILURE);
    }
    /* Bind to device */
    if (setsockopt(m_sniffSocket, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)
        {
        perror("SO_BINDTODEVICE");
        close(m_sniffSocket);
        exit(EXIT_FAILURE);
    }
    return true;
}

RawPacketsPoolItem NIC::receivePacket()
{
    struct sockaddr_storage tmp_addr;
    socklen_t len = sizeof(tmp_addr);

    //get free packet from allocated pool
    typename RawPacketsPool::value_type pack = std::move(m_nicRecvPacketsPool.getObject());

    //Read data from netork to pool item
    int rs = recvfrom(m_sniffSocket, const_cast<uint8_t *>(((ETHPacket *)pack.get())->getDataImpl()), pack->getMaxPacketSize(), 0, (struct sockaddr*)&tmp_addr, &len);
    if(rs == -1)
    {
        perror(strerror(errno));
        pack->setSize(0);
    }
    else
    {
        pack->setSize(rs);
        pack->setTimestamp();
    }

    //return pool item to processing
    return (pack);
}
