#include <time.h>
#include "ResultNotifier.h"
#include "Logger.h"

NotificationData::NotificationData(const std::string &source, AVPStorage &resp, AVPStorage &req) :
timeMsec(),
sourceId(source)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timeMsec = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    respAvp.swap(resp);
    reqAvp.swap(req);
}

std::string NotificationData::to_string() const
{
    return std::string("NotificationData [") + "time: " + std::to_string(timeMsec)
            + ", source: " + sourceId
            + ", resp: " + std::to_string(respAvp)
            + ", req: " + std::to_string(reqAvp)
            + "]";
}

ResultNotifier &ResultNotifier::instance()
{
    static ResultNotifier inst;
    return inst;
}
ResultNotifier::ResultNotifier()
{
}

ResultNotifier::~ResultNotifier()
{
}

void ResultNotifier::initialize (const std::string &sourceId)
{
    if(m_sourceId.empty())
    {
        m_sourceId = sourceId;
    }

    if(!m_notifierThread.joinable())
    {
        m_notifierThread = std::thread(&ResultNotifier::notifierThread, this);
    }
}

void ResultNotifier::deinitialize()
{
    setStop();
    m_notifierThread.join();
}

void ResultNotifier::setStop()
{
    m_stop = true;
}

void ResultNotifier::notifierThread()
{
    while(!m_stop)
    {
        NotificationDataQueueType &&data = m_queue.getObject();

       //TODO
       logger("ResultNotifier: %s\n", data->to_string().c_str());
    }
}
