#pragma once

#include <cassert>

namespace mfuse
{
    template<typename T>
    class ThreadSingleton
    {
    public:
        ThreadSingleton(T* inst);
        void Destroy(T* inst);

        static T& Get();
        static void Set(T* newInst);

    private:
        static thread_local T* singleton;
    };

    template<typename T>
    thread_local T* ThreadSingleton<T>::singleton;

    template<typename T>
    ThreadSingleton<T>::ThreadSingleton(T* inst)
    {
        singleton = inst;
    }

    template<typename T>
    void ThreadSingleton<T>::Destroy(T* inst)
    {
        if (singleton == inst) singleton = nullptr;
    }

    template<typename T>
    T& ThreadSingleton<T>::Get()
    {
        assert(singleton);
        return *singleton;
    }

    template<typename T>
    void ThreadSingleton<T>::Set(T* newInst)
    {
        singleton = newInst;
    }

    template<typename OldT, typename NewT>
    class ThreadCastSingleton
    {
    public:
        static NewT& Get();
        static void Set(NewT* newInst);
    };

    template<typename OldT, typename NewT>
    NewT& ThreadCastSingleton<OldT, NewT>::Get()
    {
        return static_cast<NewT&>(ThreadSingleton<OldT>::Get());
    }

    template<typename OldT, typename NewT>
    void ThreadCastSingleton<OldT, NewT>::Set(NewT* newInst)
    {
        ThreadSingleton<OldT>::Set(static_cast<OldT*>(newInst));
    }
}