
#ifndef RESULTNOTIFIER_H
#define RESULTNOTIFIER_H
#include "RADIUSPacket.h"
#include "ObjectQueue.h"
#include <thread>

struct NotificationData
{
    NotificationData(const std::string &sourceId, AVPStorage &resp, AVPStorage &req);
    ~NotificationData() = default;
    NotificationData(NotificationData &&src) = default;
    NotificationData &operator= (NotificationData &&src) = default;

    std::string to_string() const;
    size_t timeMsec;
    std::string sourceId;
    AVPStorage respAvp;
    AVPStorage reqAvp;
};

class ResultNotifier
{
protected:
    ResultNotifier();
public:
    ~ResultNotifier();
    static ResultNotifier &instance();

    void initialize (const std::string &sourceId);
    void deinitialize();
    void setStop();
    const std::string &getSourceId() { return m_sourceId; }
    using NotificationDataQueueType = std::unique_ptr<NotificationData>;
    using ResultNotifierQueue = ObjectQueue<NotificationDataQueueType, WaitLockPolicy/*Tets SpinLockPolicy*/>;

    template <class SpecificPacket>
    void provideDataForNotifier(SpecificPacket &resp, SpecificPacket &req)
    {
       m_queue.putObject(std::make_unique<NotificationData>(getSourceId(), resp.getAVPs(), req.getAVPs()));
    }
private:
    ResultNotifierQueue m_queue;
    std::string m_sourceId;
    bool m_stop;
    std::thread m_notifierThread;
    void notifierThread();
};

#endif /* RESULTNOTIFIER_H */

