template<class ...SupportPacketsType>
PacketRouter<SupportPacketsType...>::~PacketRouter()
{
    deinitialize();
}

template<class ...SupportPacketsType>
bool PacketRouter<SupportPacketsType...>::initialize(size_t sessionTimeoutSeconds)
{
    logger("PacketRouter::%s", __FUNCTION__);
    if(m_timerHandler == -1)
    {
        //create timer through file descriptor
        m_timerHandler= timerfd_create(CLOCK_MONOTONIC, 0);
        if(m_timerHandler == -1)
        {
            logger("PacketRouter::%s Cannot create timer", __FUNCTION__);
            perror(strerror(errno));
            return false;
        }

        //charge the timer
        struct itimerspec new_value;
        if (clock_gettime(CLOCK_MONOTONIC, &new_value.it_value) == -1)
        {
            logger("PacketRouter::%s cannot get current time", __FUNCTION__);
            perror("clock_gettime");
            return false;
        }

        new_value.it_value.tv_sec += sessionTimeoutSeconds;
        new_value.it_interval.tv_sec = sessionTimeoutSeconds;
        new_value.it_interval.tv_nsec = 0;
        if(timerfd_settime(m_timerHandler, TFD_TIMER_ABSTIME, &new_value, nullptr) == -1)
        {
            logger("PacketRouter::%s Cannot charge the sync timer", __FUNCTION__);
            perror("timerfd_settime");
            return false;
        }
        logger("PacketRouter::%s Starting sync thread", __FUNCTION__);
        m_syncronizerThread = std::thread(&PacketRouter::syncThread, this);
    }
    return true;
}

template<class ...SupportPacketsType>
void PacketRouter<SupportPacketsType...>::deinitialize()
{
    m_stopFlag = true;
    if(m_syncronizerThread.joinable())
    {
        m_syncronizerThread.join();
    }
}

template<class ...SupportPacketsType>
void PacketRouter<SupportPacketsType...>::syncThread()
{
    logger("PacketRouter::%s Starting sync thread", __FUNCTION__);
    int pollTimeout = 500; //0.5 sec
    while(!m_stopFlag)
    {
        uint64_t expirations = 0;
        int ret = 0;
        struct pollfd fd;
        fd.fd= m_timerHandler;
        fd.events = POLLIN;

        //one timer in poll
        ret = poll(&fd, 1, pollTimeout);
        if(!ret)
        {
            //timeout elapsed, not timer
            continue;
        }
        if(ret == -1)
        {
            logger("PacketRouter::%s Sync timer poll error: %s", __FUNCTION__, strerror(errno));
            continue;
        }

        //read expiration count
        ret = read(m_timerHandler, &expirations, sizeof(expirations));
        if (ret != sizeof(expirations))
        {
            logger("PacketRouter::%s Sync timer read expirations error: %s", __FUNCTION__, strerror(errno));
            continue;
        }

        //send sync command
        this->route(ControlMessageId::SYNC_TIMEOUT);
    }
}

//Template implementations
template <class ...SupportedPacketsType>
template <class Packet, class IncominPacket>
typename PacketProcessor<Packet>::PacketProcessorQueueItem
PacketRouter<SupportedPacketsType...>::createSpecificPacketPtr(IncominPacket &&incPacket)
{
    //TODO use specific pool from specific PacketProcessor, instead createPacketPtr()
    typename PacketProcessor<Packet>::PacketProcessorQueueItem specialPacket(Packet::createPacketPtr(std::move(incPacket)));
    return specialPacket;
}

//get specific packetProcessor from tuple by packet Type
template <class ...SupportedPacketsType>
template <class Packet>
PacketProcessor<Packet> &PacketRouter<SupportedPacketsType...>::getSpecificPacketProcessor()
{
    return std::get<PacketProcessor<Packet>>(m_currentProcessors);
}

//Route packet from NIC to specific packet processor instance
template <class ...SupportedPacketsType>
template <class Packet>
void PacketRouter<SupportedPacketsType...>::route(Packet &&packet)
{
    //logger("PacketRouter::%s", __FUNCTION__);
    //logger("PacketRouter::%s receive packet: %s", __FUNCTION__, packet->to_string().c_str());
    //packet inspect

    uint8_t *payload = nullptr;
    std::apply([this, &packet, &payload](auto &...x)
    {
        //1. determine packet type, supported by specific PacketProcessor
        bool needInterrupt = false;
        bool convResult[]{
                (!needInterrupt ?
                 needInterrupt = (std::remove_reference<decltype(x)>::type::PacketProcessorPacket::isConvertiblePacket(packet.get(), &payload))
                : false)...};

        //2. push packet type to specific packetProcessor instanse
        int index = 0;
        int res[]{(convResult[index++] ?
                x.pushPacket(
                        this->createSpecificPacketPtr<typename std::remove_reference<decltype(x)>::type::PacketProcessorPacket>(std::move(packet))) : 0)...};
        (void)res;
    }, m_currentProcessors);
}

//Override for ControlMessage packet types
template <class ...SupportedPacketsType>
void PacketRouter<SupportedPacketsType...>::route(ControlMessageId packet)
{
    //send command packet type to ALL
    std::apply([packet](auto &...x)
    {
        using expander = int[];
        expander {x.pushPacket(packet)...};
    }, m_currentProcessors);
}

//Just troubleshooting - get registered protocol names in our PacketRouter
template <class ...SupportedPacketsType>
std::string PacketRouter<SupportedPacketsType...>::getRegisteredProtocolNames() const
{
    //get protocol string from all packets
    std::list<std::string> ret;
    std::apply([&ret](const auto &...x)
    {
        ret.insert(ret.end(), {x.getProtocolName()...});
    }, m_currentProcessors);

    //collect all with ',' delimeter
    std::string resultStr = std::accumulate(std::next(ret.begin()), ret.end(), std::string(*ret.begin()),
                [](std::string rett, const std::string &val)
                {
                    return rett + ", " + val;
                });
    return resultStr;
}
