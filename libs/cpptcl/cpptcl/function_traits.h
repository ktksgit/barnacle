#ifndef CPPTCL_FUNCTION_TRAITS_H
#define CPPTCL_FUNCTION_TRAITS_H

#include <tuple>

template<class F>
struct FunctionTraits;

// function pointer
template<class R, class... Args>
struct FunctionTraits<R(*)(Args...)> : public FunctionTraits<R(Args...)>
{};

template<class R, class... Args>
struct FunctionTraits<R(Args...)>
{
    using return_type = R;

    static constexpr std::size_t arity = sizeof...(Args);
    using ArgsTypeList = std::tuple<Args...>;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, ArgsTypeList>::type;
    };
};

/// member function pointer
template<class C, class R, class... Args>
struct FunctionTraits<R(C::*)(Args...)> : public FunctionTraits<R(C&, Args...)>
{};

/// const member function pointer
template<class C, class R, class... Args>
struct FunctionTraits<R(C::*)(Args...) const> : public FunctionTraits<R(C&, Args...)>
{};

/// member object pointer
template<class C, class R>
struct FunctionTraits<R(C::*)> : public FunctionTraits<R(C&)>
{};

namespace {
    template<std::size_t N, typename... T, std::size_t... I>
    std::tuple<std::tuple_element_t<N + I, std::tuple<T...>>...>
        sub(std::index_sequence<I...>);

    template<std::size_t StartIdx, typename... T>
    using TupleSubpack = decltype(sub<StartIdx, T...>(std::make_index_sequence<sizeof...(T) - StartIdx>{}));
}

template<typename Functor>
struct FunctionTraits
{
private:
    using call_type = FunctionTraits<decltype(&Functor::operator())>;

public:
    using return_type = typename call_type::return_type;

    static constexpr std::size_t arity = call_type::arity - 1;
    using ArgsTypeList = TupleSubpack<1, typename call_type::ArgsTypeList>;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template argument<N + 1>::type;
    };
};

template<class F>
struct FunctionTraits<F&> : public FunctionTraits<F>
{};

template<class F>
struct FunctionTraits<F&&> : public FunctionTraits<F>
{};


#endif // CPPTCL_FUNCTION_TRAITS_H
