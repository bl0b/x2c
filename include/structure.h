#include "base.h"
#include "iter_tuple.h"
#include <type_traits>
#include <functional>
#include <iostream>

struct combination_base {};

template <class kls, typename... Elements> struct combination;


template <typename X, typename A, typename B>
struct if_is_combinable {
    typedef typename std::conditional<
        std::is_class<X>::value && std::is_base_of<combination_base, X>::value,
        A,
        B
    >::type type;
};

template <typename TransformerFactory>
struct transformer {
    template <typename Something>
        struct dest_type {
            static Something widget();
            typedef typename
                std::remove_reference<
                    decltype(TransformerFactory::transform(widget()))
                >::type type;
        };

    template <typename kls, typename... Elements>
        struct dest_type<combination<kls, Elements...>> {
            typedef combination<kls, typename dest_type<Elements>::type...> type;
        };


    template <typename Element>
        static
        auto transform(Element && x)
            -> typename dest_type<Element>::type &&
        {
            typedef typename std::enable_if<
                        !(std::is_class<Element>::value && std::is_base_of<combination_base, Element>::value),
                        TransformerFactory>::type factory;
            return std::move(factory::transform(x));
        }


    template <typename kls, typename... Elements>
        static
        auto transform(const combination<kls, Elements...> & x)
            -> typename dest_type<combination<kls, Elements...>>::type &&
        {
            DEBUG;
            typedef typename dest_type<combination<kls, Elements...>>::type return_type;
            return std::move(map_tuple<transformer<TransformerFactory>, return_type, Elements...>::transform(x));
        }

    /*template <typename X>*/
    /*static auto transform(const X & src)*/
        /*-> typename dest_type<X>::type*/
    /*{*/
        /*return transform_impl<X>::transform(src);*/
    /*}*/
/**/
    /*template <typename X>*/
    /*static auto transform(X && src)*/
        /*-> typename dest_type<X>::type &&*/
    /*{*/
        /*return std::move(transform_impl<X>::transform(src));*/
    /*}*/
/**/
    /*template <typename X>*/
    /*static auto transform(X & src)*/
        /*-> decltype(transform_impl<X>::transform(src))*/
    /*{*/
        /*return transform_impl<X>::transform(src);*/
    /*}*/
};

template <class kls, typename... Elements>
struct combination : public std::tuple<Elements...>, public virtual combination_base {
    typedef kls derived;
    typedef std::tuple<Elements...> tuple;
    /*typedef Elements elements;*/

    struct clone_helper {
        template <typename X> struct dest_type { typedef X type; };
        template <typename X> static X transform(const X& x) { return X(x); }
    };

    combination(const Elements & ... y)
        : std::tuple<Elements...>(y...)
    { debug_log << "ctor " << typeid(*this).name() << debug_endl; }

    combination(const combination<kls, Elements...>&& c)
        /*: std::tuple<Elements...>(map_tuple<clone_helper, tuple, Elements...>::transform(c))*/
        : std::tuple<Elements...>(c)
    { debug_log << "move ctor " << typeid(*this).name() << debug_endl; }

    combination(const combination<kls, Elements...>& c)
        /*: std::tuple<Elements...>(map_tuple<clone_helper, tuple, Elements...>::transform(c))*/
        : std::tuple<Elements...>(c)
    { debug_log << "copy ctor " << typeid(*this).name() << debug_endl; }

    combination(const std::tuple<Elements...>& c)
        : std::tuple<Elements...>(c)
        /*: std::tuple<Elements...>(c)*/
    { debug_log << "tuple copy ctor " << typeid(*this).name() << debug_endl; }
    /*combination(std::tuple<Elements...> && y) : std::tuple<Elements...>(y) {}*/

    template <typename IndexTuple, typename Element> struct combinator_helper;
    template <int... Index, typename Element>
        struct combinator_helper<index_tuple<Index...>, Element> {
            typedef combination<kls, Elements..., Element> return_type;

            static return_type
            combine(combination<kls, Elements...> & c, Element && e)
            {
                DEBUG;
                return (return_type(
                           std::move(std::get<Index>(c))...,
                           std::move(e)
                       ));
            }

            static return_type
            combine(combination<kls, Elements...> & c, const Element & e)
            {
                DEBUG;
                return (return_type(
                           std::get<Index>(c)...,
                           e
                       ));
            }
        };

    template <typename Element>
        combination<kls, Elements..., Element> combine(Element && e)
        {
            return combinator_helper<
                       typename make_indexes<Elements...>::type,
                       Element
                   >::combine(*this, std::move(e));
        }

    template <typename Element>
        combination<kls, Elements..., Element> combine(const Element & e)
        {
            return combinator_helper<
                       typename make_indexes<Elements...>::type,
                       Element
                   >::combine(*this, e);
        }

    template <typename IndexTuple, typename IndexTuple2, typename... Elements2> struct cat_helper;
    template <int... Index1, int... Index2, typename... Elements2>
        struct cat_helper<index_tuple<Index1...>, index_tuple<Index2...>, Elements2...> {
            typedef combination<kls, Elements..., Elements2...> return_type;

            static return_type &&
            combine(combination<kls, Elements...> & c, combination<kls, Elements2...> && e)
            {
                return std::move(return_type(
                           static_cast<Elements &&>(std::get<Index1>(c))...,
                           static_cast<Elements2 &&>(std::get<Index2>(c))...
                       ));
            }
        };

    template <typename... Elements2>
        combination<kls, Elements..., Elements2...> combine(std::tuple<Elements2...>&& t)
        {
            return cat_helper<
                       typename make_indexes<Elements...>::type,
                       typename make_indexes<Elements2...>::type,
                       Elements2...
                   >::combine(*this, std::move(t));
            /*return { std::forward(std::tuple_cat(*this, t)) };*/
            /*return { std::move(std::tuple_cat(*this, t)) };*/
        }

    template <typename Combinator, typename Evaluator>
        void apply(Combinator& c = Combinator(), Evaluator& e = Evaluator())
        {
            apply_tuple(*this, c, e);
        }

    template <typename TransformerFactory>
        typename transformer<TransformerFactory>::template dest_type<combination<kls, Elements...>>::type &&
        transform()
        {
            /*return *this;*/
            return std::move(transformer<TransformerFactory>::transform(*this));
        }
};

#define OPERATOR_DEFINITION &
#define SPECIAL_CLASS ordered_sequence
#include "operators.h"
#undef OPERATOR_DEFINITION
#undef SPECIAL_CLASS

#define OPERATOR_DEFINITION |
#define SPECIAL_CLASS alternative
#include "operators.h"
#undef OPERATOR_DEFINITION
#undef SPECIAL_CLASS

#define OPERATOR_DEFINITION ,
#define SPECIAL_CLASS unordered_sequence
#include "operators.h"
#undef OPERATOR_DEFINITION
#undef SPECIAL_CLASS

/* atoms */

template <typename Element>
combination<single, Element>
make_single(Element& e) { return { std::move(e) }; }

template <typename Element>
combination<multiple, Element>
make_multiple(Element& e) { return { std::move(e) }; }

template <typename kls, typename Element>
combination<optional<kls>, Element>
make_optional(combination<kls, Element>& e) { return { std::move(e) }; }

template <typename Element>
combination<single, Element>
make_single(Element&& e) { return { std::move(e) }; }

template <typename Element>
combination<multiple, Element>
make_multiple(Element&& e) { return { std::move(e) }; }

template <typename kls, typename Element>
combination<optional<kls>, Element>
make_optional(combination<kls, Element>&& e) { return { std::move(std::get<0>(e)) }; }

