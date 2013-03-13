#include "base.h"
#include "iter_tuple.h"
#include <type_traits>
#include <functional>
#include <iostream>

struct combination_base {};

template <class kls, typename... Items> struct combination;


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
            static_assert(!std::is_base_of<combination_base, Something>::value, "Combinable !");
            static Something widget();
            typedef typename
                std::remove_reference<
                    decltype(TransformerFactory::transform(widget()))
                >::type type;
        };

    template <typename kls, typename... Items>
        struct dest_type<combination<kls, Items...>> {
            typedef combination<kls, typename dest_type<Items>::type...> type;
        };


    template <typename Item>
        static
        auto transform(const Item & x)
            -> typename dest_type<Item>::type &&
        {
            static_assert(!(std::is_class<Item>::value && std::is_base_of<combination_base, Item>::value), "this should have never be called in this context.");
            typedef typename std::enable_if<
                        !(std::is_class<Item>::value && std::is_base_of<combination_base, Item>::value),
                        TransformerFactory>::type factory;
            return std::move(factory::transform(x));
        }


    template <typename kls, typename... Items>
        static
        auto transform(const combination<kls, Items...> & x)
            -> typename dest_type<combination<kls, Items...>>::type &&
        {
            DEBUG;
            typedef typename dest_type<combination<kls, Items...>>::type return_type;
            return std::move(map_tuple<transformer<TransformerFactory>, return_type, Items...>::transform(x));
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

template <class kls, typename... Items>
struct combination : public std::tuple<Items...>, public virtual combination_base {
    typedef kls derived;
    typedef std::tuple<Items...> tuple;
    /*typedef Items elements;*/

    struct clone_helper {
        template <typename X> struct dest_type { typedef X type; };
        template <typename X> static X transform(const X& x) { return X(x); }
    };

    combination(const Items & ... y)
        : std::tuple<Items...>(y...)
    { debug_log << "ctor " << typeid(*this).name() << debug_endl; }

#if 0
    combination(const combination<kls, Items...>&& c)
        /*: std::tuple<Items...>(map_tuple<clone_helper, tuple, Items...>::transform(c))*/
        : std::tuple<Items...>(c)
    { debug_log << "move ctor " << typeid(*this).name() << debug_endl; }
#endif

    combination(const combination<kls, Items...>& c)
        /*: std::tuple<Items...>(map_tuple<clone_helper, tuple, Items...>::transform(c))*/
        : std::tuple<Items...>(c)
    { debug_log << "copy ctor " << typeid(*this).name() << debug_endl; }

    combination(const std::tuple<Items...>& c)
        : std::tuple<Items...>(c)
        /*: std::tuple<Items...>(c)*/
    { debug_log << "tuple copy ctor " << typeid(*this).name() << debug_endl; }
    /*combination(std::tuple<Items...> && y) : std::tuple<Items...>(y) {}*/

    template <typename IndexTuple, typename Item> struct combinator_helper;
    template <int... Index, typename Item>
        struct combinator_helper<index_tuple<Index...>, Item> {
            typedef combination<kls, Items..., Item> return_type;

            static return_type
            combine(combination<kls, Items...> & c, Item && e)
            {
                DEBUG;
                return (return_type(
                           std::move(std::get<Index>(c))...,
                           std::move(e)
                       ));
            }

            static return_type
            combine(combination<kls, Items...> & c, const Item & e)
            {
                DEBUG;
                return (return_type(
                           std::get<Index>(c)...,
                           e
                       ));
            }
        };

    template <typename Item>
        combination<kls, Items..., Item> combine(Item && e)
        {
            return combinator_helper<
                       typename make_indexes<Items...>::type,
                       Item
                   >::combine(*this, std::move(e));
        }

    template <typename Item>
        combination<kls, Items..., Item> combine(const Item & e)
        {
            return combinator_helper<
                       typename make_indexes<Items...>::type,
                       Item
                   >::combine(*this, e);
        }

    template <typename IndexTuple, typename IndexTuple2, typename... Items2> struct cat_helper;
    template <int... Index1, int... Index2, typename... Items2>
        struct cat_helper<index_tuple<Index1...>, index_tuple<Index2...>, Items2...> {
            typedef combination<kls, Items..., Items2...> return_type;

            static return_type &&
            combine(combination<kls, Items...> & c, combination<kls, Items2...> && e)
            {
                return std::move(return_type(
                           static_cast<Items &&>(std::get<Index1>(c))...,
                           static_cast<Items2 &&>(std::get<Index2>(c))...
                       ));
            }
        };

    template <typename... Items2>
        combination<kls, Items..., Items2...> combine(std::tuple<Items2...>&& t)
        {
            return cat_helper<
                       typename make_indexes<Items...>::type,
                       typename make_indexes<Items2...>::type,
                       Items2...
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
        typename transformer<TransformerFactory>::template dest_type<combination<kls, Items...>>::type &&
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

template <typename Item>
combination<single, Item>
make_single(const Item& e) { return { e }; }

template <typename Item>
combination<multiple, Item>
make_multiple(const Item& e) { return { e }; }

template <typename kls, typename Item>
combination<optional<kls>, Item>
make_optional(const combination<kls, Item>& e) { return { e }; }

template <typename Item>
combination<single, Item>
make_single(Item&& e) { return { std::move(e) }; }

template <typename Item>
combination<multiple, Item>
make_multiple(Item&& e) { return { std::move(e) }; }

template <typename kls, typename Item>
combination<optional<kls>, Item>
make_optional(combination<kls, Item>&& e) { return { std::move(std::get<0>(e)) }; }

