#ifndef COMMONDATADEFINITION_H
#define COMMONDATADEFINITION_H
#include "Utils/CommonObjectPool.h"
#include "Packets/RawPacket.h"

template<class Object>
using LockFreePool = ObjectPool<Object, SpinLockPolicy>;

//Initial pool for raw packets
using RawPacketsPool = LockFreePool<ETHPacket>;
using RawPacketsPoolItem = typename RawPacketsPool::value_type;

#endif /* COMMONDATADEFINITION_H */
