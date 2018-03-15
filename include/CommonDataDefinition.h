#ifndef COMMONDATADEFINITION_H
#define COMMONDATADEFINITION_H
#include "CommonObjectPool.h"
#include "RawPacket.h"

template<class Object>
using LockFreePool = ObjectPool<Object, SpinLockPolicy>;

//Initial pool for raw packets
using RawPacketsPool = LockFreePool<ETHPacket>;
using RawPacketsPoolItem = typename RawPacketsPool::value_type;

//TODO remove ?
template<class T, class Deleter>
using RecvPacketNew = std::unique_ptr<T, Deleter>;


#endif /* COMMONDATADEFINITION_H */

