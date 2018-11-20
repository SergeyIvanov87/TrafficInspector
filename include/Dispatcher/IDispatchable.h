#ifndef IDISPATCHABLE_H
#define IDISPATCHABLE_H

template <class Impl>
struct IDispatchable
{
    template<class Source, class ...SourceAdditionalData>
    static bool isDispatchableType(const Source &pSource, SourceAdditionalData &&...additionalData)
    {
        return Impl::isDispatchableTypeImpl(pSource, std::forward<SourceAdditionalData>(additionalData)...);
    }
};
#endif //IDISPATCHABLE_H
