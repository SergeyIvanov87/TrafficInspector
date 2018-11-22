#ifndef LOCKPOLICIES_H
#define LOCKPOLICIES_H

#include <mutex>
#include <condition_variable>

class WaitLockPolicy
{
public:
    using LockGuard = std::unique_lock<std::mutex>;
    LockGuard lock()
    {
        return std::unique_lock<std::mutex>(m_mutex);
    }
    
    template<class Predicate>
    void wait(LockGuard &lock, Predicate pred)
    {
        m_cond.wait(lock, pred);
    }
    void notifyWait()
    {
        m_cond.notify_one();
    }
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
 };

///////////////////
#include <atomic>
#include <thread>
#include <chrono>

class SpinLock
{
    std::atomic_flag locked = ATOMIC_FLAG_INIT ;
public:
    void lock()
    {
        while (locked.test_and_set(std::memory_order_acquire)) { ; }
    }
    void unlock()
    {
        locked.clear(std::memory_order_release);
    }
};

class SpinLockPolicy
{
public:
    using LockGuard = std::unique_lock<SpinLock>;
    LockGuard lock()
    {
        return std::unique_lock<SpinLock>(m_lock);
    }
    
    template<class Predicate>
    void wait(LockGuard &lock, Predicate pred)
    {
        while(!pred())
        {
            lock.unlock();
            std::this_thread::yield();
            lock.lock();
        }
    }
    void notifyWait()
    {
    }
    
private:
    SpinLock m_lock;
    
};
#endif /* LOCKPOLICIES_H */

