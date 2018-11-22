#ifndef OBJECTQUEUE_H
#define OBJECTQUEUE_H

#include <list>
#include <cassert>
#include <iterator>
#include <memory>
#include <algorithm>
#include "LockPolicies.h"

template <class Object, class LockPolicy>
struct ObjectQueue : LockPolicy
{
    using value_type = Object;
    using Self = ObjectQueue< Object, LockPolicy>;
    using QueueContainer = std::list<Object>;

    QueueContainer m_container;
    Object getObject()
    {
        {
            auto lock = LockPolicy::lock();
            LockPolicy::wait(lock, [this](){ return !m_container.empty(); });
            Object ret = std::move(m_container.front());
            m_container.pop_front();
            return (ret);
        }
    }

    void putObject(Object &&obj)
    {
        {
            auto lock = LockPolicy::lock();
            m_container.push_back(std::move(obj));
            LockPolicy::notifyWait();
        }
    }
};
#endif /* OBJECTQUEUE_H */

