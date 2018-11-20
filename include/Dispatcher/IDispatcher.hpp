#ifndef IDISPATCHER_HPP
#define IDISPATCHER_HPP
#include <algorithm>
#include "IDispatcher.h"

#define ARGS_DECL  class Type2Dispatcher, class ...Dispatchers
#define ARGS_DEF  Type2Dispatcher, Dispatchers...

template <ARGS_DECL>
IDispatcher<ARGS_DEF>::IDispatcher(Dispatchers &&...args):
 m_dispatchers(std::forward<Dispatchers>(args)...)
{
}

template <ARGS_DECL>
template<class Type, class ...TypeAdditionalInfo>
std::optional<size_t> IDispatcher<ARGS_DEF>::canBeDispatchered(const Type &inst, TypeAdditionalInfo &&...outInfo)
{
    std::optional<size_t> dispatcherIndex;

    //1. check own base type at first
    if (!Type2Dispatcher::isDispatchableType(inst.get(), outInfo...))
    {
        return dispatcherIndex;
    }
    
    std::apply(
    [this, &dispatcherIndex, &inst, &outInfo...]
    (auto &...x)
    {
        //2. determine type, supported by specific Dispatchers instance
        bool dispatcheredByCurrentInstance = false;
        bool dispatchingResult[]
        {
                (!dispatcheredByCurrentInstance ?
                 dispatcheredByCurrentInstance = x.canBeDispatchered(inst, outInfo...).has_value()
                : false)...
        };

        //3. set Dispatcher Index, if condition happened
        if(auto it = std::find(std::begin(dispatchingResult), std::end(dispatchingResult), true);
                it !=  std::end(dispatchingResult))
        {
            dispatcherIndex = std::distance(std::begin(dispatchingResult), it);
        }
/*
        //2. push packet type to specific Dispatchers instanse
        int index = 0;
        int res[]
        {
            (dispatchingResult[index++] ?
                x.pushPacket(
                        this->createSpecificPacketPtr<typename std::remove_reference<decltype(x)>::type::PacketProcessorPacket>(std::move(inst)))
                : 0)...
        };
        (void)res;
*/
    }, m_dispatchers);
    return dispatcherIndex;
}

template <ARGS_DECL>
template<class Type>
bool IDispatcher<ARGS_DEF>::dispatch(Type &&inst)
{
    //TODO
    unsigned char *dummy = nullptr;
    std::optional<size_t> dispatcherIndex = canBeDispatchered(inst, &dummy);
    return (dispatcherIndex.has_value() ?
                dispatchByIndex(dispatcherIndex.value(), std::move(inst)) :
                false
            );
}

template <ARGS_DECL>
template<class Type>
void IDispatcher<ARGS_DEF>::dispatchByIndex(size_t dispatcherIndex, Type &&inst)
{
    std::apply([this, dispatcherIndex, &inst](auto &...x)
    {
        size_t currIndex = 0;
        bool dispatcheredByCurrentInstance = false;
        bool dispatchingResult[]
        {
                (currIndex++ == dispatcherIndex ?
                    x.dispatch(std::move(inst))
                :
                    false)...
        };
    }, m_dispatchers);
}

template <ARGS_DECL>
template<class Type, size_t dispatcherIndex>
void IDispatcher<ARGS_DEF>::dispatchByIndex(Type &&inst)
{
    //-S- TODO static_assert(false, "not yet implemented");
}
#endif //IDISPATCHER_HPP
