#ifndef IDISPATCHER_HPP
#define IDISPATCHER_HPP
#include <algorithm>
#include <iterator>
#include <array>
#include "IDispatcher.h"

#define ARGS_DECL  class Type2Dispatcher, class AtLeastOneDispatcher, class ...Dispatchers
#define ARGS_DEF  Type2Dispatcher, AtLeastOneDispatcher, Dispatchers...

template <ARGS_DECL>
IDispatcher<ARGS_DEF>::IDispatcher(AtLeastOneDispatcher &&disp, Dispatchers &&...args):
 m_dispatchers(std::forward<AtLeastOneDispatcher>(disp), std::forward<Dispatchers>(args)...)
{
}

template <ARGS_DECL>
template<class Type, class ...TypeAdditionalInfo>
std::optional<size_t> IDispatcher<ARGS_DEF>::canBeDispatchered(const Type &inst, TypeAdditionalInfo &&...outInfo)
{
    std::optional<size_t> dispatcherIndex;

    //1. check own base type at first
    if (!Type2Dispatcher::isDispatchableType(inst, outInfo...))
    {
        return dispatcherIndex;
    }

    std::apply(
    [this, &dispatcherIndex, &inst, &outInfo...]
    (auto &...x)
    {
        //2. determine type, supported by specific Dispatchers instance
        bool dispatcheredByCurrentInstance = false;
        std::array<bool, sizeof...(x)> dispatchingResult
        {
                (!dispatcheredByCurrentInstance ?
                 dispatcheredByCurrentInstance = x.canBeDispatchered(inst, outInfo...).has_value(), dispatcheredByCurrentInstance
                : false)...
        };

        //3. set Dispatcher Index, if condition happened
        if(auto it = std::find(std::begin(dispatchingResult), std::end(dispatchingResult), true);
                it !=  std::end(dispatchingResult))
        {
            dispatcherIndex = std::distance(std::begin(dispatchingResult), it);
        }

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
                dispatchByIndex(dispatcherIndex.value(), std::forward<Type>(inst)), true :
                false
            );
}

template <ARGS_DECL>
template<class Type>
size_t IDispatcher<ARGS_DEF>::dispatchBroadcast(Type &&inst)
{
    std::apply(
        [this, &inst]
        (auto &...x)
        {
            bool dispatchingResult[]
            {
                x.dispatchServiceMessage(std::forward<Type>(inst))...
            };
            (void)dispatchingResult;
        },
    m_dispatchers);
    return std::tuple_size_v<SpecificDispatchers>;
}

template <ARGS_DECL>
template<class Type>
void IDispatcher<ARGS_DEF>::dispatchByIndex(size_t dispatcherIndex, Type &&inst)
{
    std::apply([this, dispatcherIndex, &inst](auto &...x)
    {
        size_t currIndex = 0;
        bool dispatchingResult[]
        {
                (currIndex++ == dispatcherIndex ?
                    x.dispatch(std::forward<Type>(inst))
                :
                    false)...
        };
        (void)dispatchingResult;
    }, m_dispatchers);
}

template <ARGS_DECL>
template<class Type, size_t dispatcherIndex>
void IDispatcher<ARGS_DEF>::dispatchByIndex(Type &&inst)
{
    //-S- TODO static_assert(false, "not yet implemented");
}

template <ARGS_DECL>
constexpr std::string IDispatcher<ARGS_DEF>::to_string() const
{
    //get protocol string from all packets
    std::list<std::string> ret;
    std::apply([&ret](const auto &...x)
    {
        ret.insert(ret.end(), {x.to_string()...});
    }, m_dispatchers);

    //collect all with ',' delimeter
    std::string resultStr = std::accumulate(std::next(ret.begin()), ret.end(), std::string(*ret.begin()),
                [](std::string rett, const std::string &val)
                {
                    return rett + ", " + val;
                });
    return resultStr;
}

////////////////////////////////////////////////////////////////////////
//Terminator Dispatcher CRTP Interface
#define ARGS_DECL_T  class Packet, template<class> class Dispatcher
#define ARGS_DEF_T   Packet, Dispatcher<Packet>//typename Dispatcher::ProcessingType, Dispatcher

template <ARGS_DECL_T>
template<class Type, class ...TypeAdditionalInfo>
std::optional<size_t> IDispatcher<ARGS_DEF_T>::canBeDispatchered(const Type &inst, TypeAdditionalInfo &&...outInfo)
{
    return static_cast<Dispatcher<Packet> *>(this)->canBeDispatcheredImpl(
                                                inst, 
                                                std::forward<TypeAdditionalInfo>(outInfo)...);
}

template <ARGS_DECL_T>
    template<class Type>
    bool IDispatcher<ARGS_DEF_T>::dispatch(Type &&inst)
    {
        return static_cast<Dispatcher<Packet> *>(this)->onDispatchImpl(std::move(inst));
    }


template <ARGS_DECL_T>
template<class Type>
size_t IDispatcher<ARGS_DEF_T>::dispatchBroadcast(Type &&inst)
{
    return dispatchServiceMessage(std::forward<Type>(inst));
}
    
template <ARGS_DECL_T>
template<class Type>
void IDispatcher<ARGS_DEF_T>::dispatchByIndex(size_t dispatcherIndex, Type &&inst)
{
    dispatch(std::forward<Type>(inst));
}
    
template <ARGS_DECL_T>
    template<class Type>
    size_t IDispatcher<ARGS_DEF_T>::dispatchServiceMessage(Type &&inst)
    {
        return static_cast<Dispatcher<Packet> *>(this)->onDispatchBroadcastImpl(std::move(inst));
    }
    
template <ARGS_DECL_T>
    constexpr std::string IDispatcher<ARGS_DEF_T>::to_string() const
    {
        return static_cast<const Dispatcher<Packet> *>(this)->to_stringImpl();
    }

#undef ARGS_DEF_T
#undef ARGS_DECL_T

#undef ARGS_DEF
#undef ARGS_DECL
#endif //IDISPATCHER_HPP
