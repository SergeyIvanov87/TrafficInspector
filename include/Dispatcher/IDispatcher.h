#ifndef IDISPATCHER_H
#define IDISPATCHER_H
#include <tuple>
#include <utility>
#include <optional>

template <class Type2Dispatcher, class AtLeastOneDispatcher, class ...Dispatchers>
struct IDispatcher
{
    using DispatcheredType = Type2Dispatcher;
    using SpecificDispatchers = std::tuple<AtLeastOneDispatcher, Dispatchers...>;
   
    IDispatcher(AtLeastOneDispatcher &&disp, Dispatchers &&...dispatchers);

    template<class Type, class ...DispatcheredInfo>
    std::optional<size_t> canBeDispatchered(const Type &inst, DispatcheredInfo &&...outInfo);

    template<class Type>
    bool dispatch(Type &&inst);

    template<class Type>
    void dispatchByIndex(size_t dispatcherIndex, Type &&inst);

    template<class Type>
    size_t dispatchBroadcast(Type &&inst);
    
    template<class Type, size_t dispatcherIndex>
    void dispatchByIndex(Type &&inst);

    constexpr std::string to_string() const;
private:
    SpecificDispatchers m_dispatchers;
};

//Terminator Dispatcher CRTP Interface
template <class Packet, template<class> class Dispatcher>
struct IDispatcher<Packet, Dispatcher<Packet>>
{
    using DispatcheredType = Packet;
    template<class Type, class ...TypeAdditionalInfo>
    std::optional<size_t> canBeDispatchered(const Type &inst, TypeAdditionalInfo &&...outInfo);

    template<class Type>
    bool dispatch(Type &&inst);

    template<class Type>
    size_t dispatchBroadcast(Type &&inst);

    template<class Type>
    void dispatchByIndex(size_t dispatcherIndex, Type &&inst);
    
    template<class Type>
    size_t dispatchServiceMessage(Type &&inst);

    constexpr std::string to_string() const;
};

#endif //IDISPATCHER_H
