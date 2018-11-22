
#ifndef COMMONOBJECTPOOL_H
#define COMMONOBJECTPOOL_H
#include "ObjectQueue.h"

template<class PoolImpl>
class ObjectPoolItemDeleter
{
public:
    ObjectPoolItemDeleter(PoolImpl *pool):
        m_parent(pool)
    {
    }

    template<class ReleasedObject>
    void operator() (ReleasedObject *ptr)
    {
        if(ptr && m_parent)
        {
            m_parent->releaseObject(ptr);
        }
        else
        {
            assert(false);
        }

    }
    PoolImpl *m_parent;
};

template<class Object, class LockPolicy>
class ObjectPool : public ObjectQueue<
                                    std::unique_ptr<
                                                    Object,
                                                    ObjectPoolItemDeleter<
                                                                           ObjectPool<Object, LockPolicy>
                                                                         >
                                                    >,
                                    LockPolicy>
{
public:
    using NativeObject = Object;
    using Self = ObjectPool<Object, LockPolicy>;
    using NativeObjectPoolItemReleaser = ObjectPoolItemDeleter<ObjectPool<Object, LockPolicy>>;
    using NativeObjectPoolItemWrapper = std::unique_ptr<NativeObject, NativeObjectPoolItemReleaser>;
    using Base = ObjectQueue<NativeObjectPoolItemWrapper, LockPolicy>;
    friend class ObjectPoolItemDeleter<ObjectPool<Object, LockPolicy>>;

    using value_type = typename Base::value_type;
    using PoolContainer = typename Base::QueueContainer;

    ObjectPool(size_t size) : PoolSize(size)
    {
        std::generate_n(std::back_inserter(Base::m_container),  PoolSize, [this]()
        {
            return NativeObjectPoolItemWrapper(new Object(), NativeObjectPoolItemReleaser((this)));
        });
    }
    ObjectPool() = delete;
    ~ObjectPool()
    {
        //TODO wait all object returns
        auto guard = LockPolicy::lock();
        Base::m_container.clear();
    }

protected:
    void releaseObject(Object *objPtr)
    {
        value_type poolObject(std::move(objPtr), NativeObjectPoolItemReleaser((this)));
        this->putObject(std::move(poolObject));
    }

    size_t PoolSize;
};
#endif /* COMMONOBJECTPOOL_H */

