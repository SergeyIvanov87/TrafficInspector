#define ARGS_DECL  class ...SupportPacketsType
#define ARGS_DEF  SupportPacketsType...


template<ARGS_DECL>
PacketDispatcher<ARGS_DEF>::~PacketDispatcher()
{
    deinitialize();
}

template<ARGS_DECL>
bool PacketDispatcher<ARGS_DEF>::initialize(size_t sessionTimeoutSeconds)
{
    logger("PacketDispatcher::%s", __FUNCTION__);
    if(m_timerHandler == -1)
    {
        //create timer through file descriptor
        m_timerHandler= timerfd_create(CLOCK_MONOTONIC, 0);
        if(m_timerHandler == -1)
        {
            logger("PacketDispatcher::%s Cannot create timer", __FUNCTION__);
            perror(strerror(errno));
            return false;
        }

        //charge the timer
        struct itimerspec new_value;
        if (clock_gettime(CLOCK_MONOTONIC, &new_value.it_value) == -1)
        {
            logger("PacketDispatcher::%s cannot get current time", __FUNCTION__);
            perror("clock_gettime");
            return false;
        }

        new_value.it_value.tv_sec += sessionTimeoutSeconds;
        new_value.it_interval.tv_sec = sessionTimeoutSeconds;
        new_value.it_interval.tv_nsec = 0;
        if(timerfd_settime(m_timerHandler, TFD_TIMER_ABSTIME, &new_value, nullptr) == -1)
        {
            logger("PacketDispatcher::%s Cannot charge the sync timer", __FUNCTION__);
            perror("timerfd_settime");
            return false;
        }
        logger("PacketRouter::%s Starting sync thread", __FUNCTION__);
        m_syncronizerThread = std::thread(&PacketDispatcher::syncThread, this);
    }
    return true;
}

template<ARGS_DECL>
void PacketDispatcher<ARGS_DEF>::deinitialize()
{
    m_stopFlag = true;
    if(m_syncronizerThread.joinable())
    {
        m_syncronizerThread.join();
    }
}

template<ARGS_DECL>
void PacketDispatcher<ARGS_DEF>::syncThread()
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

//Route packet from NIC to specific packet processor instance
template<ARGS_DECL>
template <class Packet>
std::optional<size_t> PacketDispatcher<ARGS_DEF>::route(Packet &&packet)
{
    std::optional<size_t> dispatcherIndex;
    std::optional<size_t> dispatcherIndexInstanceNum;

    std::apply([this, &packet, &dispatcherIndex, &dispatcherIndexInstanceNum](auto &...x)
    {
        uint8_t *payloadPtr = nullptr;
        std::array<bool, sizeof...(x)> dispatchingResult
        {
                (!dispatcherIndexInstanceNum.has_value() ?
                 dispatcherIndexInstanceNum = x.canBeDispatchered(*packet.get(), &payloadPtr),dispatcherIndexInstanceNum.has_value()
                : false)...
        };

        int index = 0;
        int res[]
        {
            (dispatchingResult[index++] ?
                x.dispatchByIndex(dispatcherIndexInstanceNum.value(), std::forward<Packet>(packet)), index
            : 0)...
        };
            (void) res;
            
        //get dispatcher index
        /*
        if(auto it = std::find(std::begin(dispatchingResult), std::end(dispatchingResult), true);
                it !=  std::end(dispatchingResult))
        {
            dispatcherIndex = std::distance(std::begin(dispatchingResult), it);

            //
            int index = 0;
            int res[]
            {
                (dispatchingResult[index++] ?
                    x.dispatch(std::forward<Packet>(packet)), index
                : 0)...
            };
            (void) res;
        }
        * */
    }, m_currentDispatchers);

    return dispatcherIndex;
}

//Override for ControlMessage packet types
template<ARGS_DECL>
size_t PacketDispatcher<ARGS_DEF>::route(ControlMessageId packet)
{
    //send command packet type to ALL
    std::apply([packet](auto &...x)
    {
        using expander = size_t[];
        expander {x.dispatchBroadcast(packet)...};
    }, m_currentDispatchers);

    return std::tuple_size_v<DispatchersPools>;
}

//Just troubleshooting - get registered protocol names in our PacketRouter
template<ARGS_DECL>
std::string PacketDispatcher<ARGS_DEF>::getRegisteredProtocolNames() const
{
    //get protocol string from all packets
    std::list<std::string> ret;
    std::apply([&ret](const auto &...x)
    {
        ret.insert(ret.end(), {x.to_string()...});
    }, m_currentDispatchers);

    //collect all with ',' delimeter
    std::string resultStr = std::accumulate(std::next(ret.begin()), ret.end(), std::string(*ret.begin()),
                [](std::string rett, const std::string &val)
                {
                    return rett + ", " + val;
                });
    return resultStr;
}
#undef ARGS_DEF
#undef ARGS_DECL
