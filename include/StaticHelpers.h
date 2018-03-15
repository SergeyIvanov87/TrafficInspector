template <class F, class ...Params>
bool InterruptWrapper(F f, bool &needInterrupt, Params ...args)
{
    if(needInterrupt)
    {
        return false;
    }
    bool res = f(args...);
    if(res != true)
    {
        needInterrupt = true;
    }
    return res;
}
