/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef _X2C_ITER_TUPLE_H_
#define _X2C_ITER_TUPLE_H_

#include <tuple>
#include <iostream>

template <int I, int TSize, typename Tuple>
struct iterate_over_tuple_impl
    : public iterate_over_tuple_impl<I + 1, TSize, Tuple>
{
    template <typename Function>
        void operator () (Function& f, const Tuple& t)
        {
            f(std::get<I>(t));
            iterate_over_tuple_impl<I + 1, TSize, Tuple>::operator () (f, t);
        }

    template <typename Function>
        void operator () (Function& f, Tuple& t)
        {
            f(std::get<I>(t));
            iterate_over_tuple_impl<I + 1, TSize, Tuple>::operator () (f, t);
        }
};

template <int I, typename Tuple>
struct iterate_over_tuple_impl<I, I, Tuple> {
    template <typename Function>
        void operator () (Function& f, const Tuple& t)
        {
            (void)f;
            (void)t;
        }
};

template <typename Function, typename... Args>
void iterate_over_tuple(Function& f, const std::tuple<Args...>& t)
{
    iterate_over_tuple_impl<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}


template <typename Function, typename... Args>
void iterate_over_tuple(Function& f, std::tuple<Args...>& t)
{
    iterate_over_tuple_impl<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}


template <int I, int TSize, typename Tuple>
struct iterate_over_tuple_with_index_impl
    : public iterate_over_tuple_impl<I + 1, TSize, Tuple>
{
    template <typename Function>
        void operator () (Function& f, Tuple& t)
        {
            f(I, std::get<I>(t));
            iterate_over_tuple_with_index_impl<I + 1, TSize, Tuple>() (f, t);
        }
    template <typename Function>
        void operator () (Function& f, const Tuple& t)
        {
            f(I, std::get<I>(t));
            iterate_over_tuple_with_index_impl<I + 1, TSize, Tuple>() (f, t);
        }
};

template <int I, typename Tuple>
struct iterate_over_tuple_with_index_impl<I, I, Tuple> {
    template <typename Function> void operator () (Function& f, Tuple& t)
    { (void)f; (void)t; }
    template <typename Function> void operator () (Function& f, const Tuple& t)
    { (void)f; (void)t; }
};

template <typename Function, typename... Args>
void iterate_over_tuple_with_index(Function& f, const std::tuple<Args...>& t)
{
    iterate_over_tuple_with_index_impl<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}

template <typename Function, typename... Args>
void iterate_over_tuple_with_index(Function& f, std::tuple<Args...>& t)
{
    iterate_over_tuple_with_index_impl<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}


template <typename Combinator, typename Evaluator>
struct applier_t {
    Combinator& c;
    Evaluator& e;
    template <typename T> void operator () (T& x) { c(e(x)); }
};

template <typename... Args, typename Combinator, typename Evaluator>
Combinator& apply_tuple(std::tuple<Args...>& t, Combinator& c = Combinator(), Evaluator& e = Evaluator())
{
    applier_t<Combinator, Evaluator> applier = { c, e };
    iterate_over_tuple(applier, t);
    return c;
}


template <int... I> struct index_tuple;

template <int I, typename IndexTuple, typename... Types>
    struct make_indexes_impl;

template <int I, int... Indexes>
    struct make_indexes_impl<I, index_tuple<Indexes...>> {
        typedef index_tuple<Indexes...> type;
        /*enum { I=I };*/
    };

template <int I, int... Indexes, typename T, typename... Types>
    struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...> {
        typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type;
    };

template <typename... Types>
    struct make_indexes : public make_indexes_impl<0, index_tuple<>, Types...> {};


template <typename Factory, typename ReturnType, typename... Types>
struct map_tuple {
    template <typename IndexTuple> struct helper;
    template <int... Indexes>
        struct helper<index_tuple<Indexes...>> {
            static ReturnType transform(const std::tuple<Types...> & src)
            {
                DEBUG;
                /*return { Factory::transform(std::get<Indexes>(src))... };*/
                return std::move(ReturnType(Factory::transform(std::get<Indexes>(src))...));
            }
        };


    static ReturnType transform(const std::tuple<Types...> & src)
    {

        DEBUG;
        return std::move(helper<typename make_indexes<Types...>::type>
        /*return helper<typename make_indexes<Types...>::type>*/
                ::transform(src));
    }
};


#endif

