#ifndef IDISPATCHER_H
#define IDISPATCHER_H
#include <tuple>
#include <optional>

template <class Type2Dispatcher, class ...Dispatchers>
struct IDispatcher
{
    using DispatcheredType = Type2Dispatcher;
    using SpecificDispatchers = std::tuple<Dispatchers...>;
    
    IDispatcher(Dispatchers &&...dispatchers);
    ~IDispatcher() = default;

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

    constexpr std::string getDescription() const;
private:
    SpecificDispatchers m_dispatchers;
};

//Terminator Dispatcher CRTP Interface
template <class Dispatcher>
struct IDispatcher<typename Dispatcher::ProcessingType, Dispatcher> :
    public Dispatcher
{
    using DispatcheredType = typename Dispatcher::ProcessingType;
    IDispatcher() = default;
    ~IDispatcher() = default;

    template<class Type, class ...TypeAdditionalInfo>
    std::optional<size_t> canBeDispatchered(const Type &inst, TypeAdditionalInfo &&...outInfo)
    {
        return static_cast<Dispatcher *>(this)->canBeDispatcheredImpl(
                                                    inst, 
                                                    std::forward<TypeAdditionalInfo>(outInfo)...);
    }

    template<class Type>
    void dispatch(Type &&inst)
    {
        return static_cast<Dispatcher *>(this)->onDispatchImpl(std::move(inst));
    }

    template<class Type>
    void dispatchServiceMessage(Type &&inst)
    {
        return static_cast<Dispatcher *>(this)->onDispatchBroadcastImpl(std::move(inst));
    }
};

#endif //IDISPATCHER_H
