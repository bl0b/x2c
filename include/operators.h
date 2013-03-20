/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#if defined(OPERATOR_DEFINITION) && defined(SPECIAL_CLASS)

#include <iostream>

template <typename K1, typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K1, E1...>, combination<K2, E2...>>
operator OPERATOR_DEFINITION (const combination<K1, E1...> & a, const combination<K2, E2...> & b)
{
    DEBUG;
    return { a, b };
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, E1..., combination<K2, E2...>>
operator OPERATOR_DEFINITION (const combination<SPECIAL_CLASS, E1...> & a, const combination<K2, E2...> & b)
{
    DEBUG;
    return a.combine(b);
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K2, E2...>, E1...>
operator OPERATOR_DEFINITION (const combination<K2, E2...> & a, const combination<SPECIAL_CLASS, E1...> & b)
{
    DEBUG;
    return combination<SPECIAL_CLASS, combination<K2, E2...>>(a).combine(b);
}

#if 0
template <typename K1, typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K1, E1...>, combination<K2, E2...>>
operator OPERATOR_DEFINITION (combination<K1, E1...> && a, const combination<K2, E2...> & b)
{
    DEBUG;
    return { a, b };
}

template <typename K1, typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K1, E1...>, combination<K2, E2...>>
operator OPERATOR_DEFINITION (const combination<K1, E1...> & a, combination<K2, E2...> && b)
{
    DEBUG;
    return { a, b };
}

template <typename K1, typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K1, E1...>, combination<K2, E2...>>
operator OPERATOR_DEFINITION (combination<K1, E1...> && a, combination<K2, E2...> && b)
{
    DEBUG;
    return { std::move(a), std::move(b) };
}
#endif

#if 0
template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, E1..., combination<K2, E2...>>
operator OPERATOR_DEFINITION (combination<SPECIAL_CLASS, E1...> && a, const combination<K2, E2...> & b)
{
    DEBUG;
    return a.combine(b);
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, E1..., combination<K2, E2...>>
operator OPERATOR_DEFINITION (const combination<SPECIAL_CLASS, E1...> & a, combination<K2, E2...> && b)
{
    DEBUG;
    return a.combine(b);
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, E1..., combination<K2, E2...>>
operator OPERATOR_DEFINITION (combination<SPECIAL_CLASS, E1...> && a, combination<K2, E2...> && b)
{
    DEBUG;
    return a.combine(std::move(b));
}
#endif

#if 0
template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K2, E2...>, E1...>
operator OPERATOR_DEFINITION (combination<K2, E2...> && a, const combination<SPECIAL_CLASS, E1...> & b)
{
    DEBUG;
    return combination<SPECIAL_CLASS, combination<K2, E2...>>(a).combine(b);
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K2, E2...>, E1...>
operator OPERATOR_DEFINITION (const combination<K2, E2...> & a, combination<SPECIAL_CLASS, E1...> && b)
{
    DEBUG;
    return combination<SPECIAL_CLASS, combination<K2, E2...>>(a).combine(b);
}

template <typename... E1, typename K2, typename... E2>
constexpr combination<SPECIAL_CLASS, combination<K2, E2...>, E1...>
operator OPERATOR_DEFINITION (combination<K2, E2...> && a, combination<SPECIAL_CLASS, E1...> && b)
{
    DEBUG;
    return combination<SPECIAL_CLASS, combination<K2, E2...>>(a).combine(b);
}
#endif

#endif

