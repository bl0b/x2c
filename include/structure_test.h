#include "base.h"
#include "iter_tuple.h"
#include <type_traits>
#include <functional>

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
#if 1
    struct simple_type {
        template <typename Element>
            struct dest_type {
                static Element && widget();
                typedef typename
                    std::remove_reference<
                        decltype(TransformerFactory::transform(widget()))
                    >::type type;
            };
    };

    struct comb_type {
        template <typename X> struct dest_type { typedef void type; };

        template <class kls, typename... Elements>
            struct dest_type<combination<kls, Elements...>> {
                typedef combination<
                    kls,
                    typename transformer<TransformerFactory>::template dest_type<Elements>::type...
                > type;
            };
    };

    template <typename Something>
        struct dest_type {
            static Something widget();
            typedef typename
                if_is_combinable<Something,
                    comb_type,
                    simple_type
                >::type::template dest_type<Something>::type type;
        };
#else
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
#endif

    template <typename Element>
        struct transform_impl {
            /*static typename dest_type<Element>::type &&*/
            /*transform(Element && element)*/
            /*{*/
                /*return static_cast<typename dest_type<Element>::type &&>(TransformerFactory::transform(element));*/
            /*}*/

            static typename dest_type<Element>::type
            transform(const Element & element)
            {
                return TransformerFactory::transform(element);
            }
        };

    template <typename kls, typename... Elements>
        struct transform_impl<combination<kls, Elements...>> {
            typedef typename transformer<TransformerFactory>
                    ::template dest_type<combination<kls, Elements...>>
                    ::type output_type;

            template <typename IndexTuple, typename DestType>
                struct transform_tuple_impl;

            template <int... Indexes, typename... Dest>
                struct transform_tuple_impl<index_tuple<Indexes...>, combination<kls, Dest...>> {
                    static
                    combination<kls, Dest...> &&
                    transform(combination<kls, Elements...> && comb)
                    {
                        /*return { std::forward<typename dest_type<Elements>::type>(*/
                        /*return { std::forward(*/
                        return std::move(combination<kls, Dest...>(
                                   static_cast<Dest &&>(
                                       transformer<TransformerFactory>::transform(
                                           std::get<Indexes>(comb)))...
                                   ));
                                                /*comb)))...) };*/
                    }

                };

            static output_type transform(const combination<kls, Elements...> & comb)
            {
                return static_cast<output_type &&>(
                        transform_tuple_impl<typename make_indexes<Elements...>::type,
                                             typename dest_type<combination<kls, Elements...>>::type>
                            ::transform(static_cast<combination<kls, Elements...> &&>(combination<kls, Elements...>(comb))));
            }

            static output_type && transform(combination<kls, Elements...> && comb)
            {
                return static_cast<output_type &&>(
                        transform_tuple_impl<typename make_indexes<Elements...>::type,
                                             typename dest_type<combination<kls, Elements...>>::type>
                            ::transform(static_cast<combination<kls, Elements...> &&>(comb)));
            }
        };


    template <typename Element>
        static
        auto transform(Element && x)
            -> typename dest_type<Element>::type
        {
            typedef typename std::enable_if<
                        !(std::is_class<Element>::value && std::is_base_of<combination_base, Element>::value),
                        TransformerFactory>::type factory;
            return factory::transform(x);
        }

    template <typename kls, typename... Elements>
        struct transform_tuple {
            typedef typename transformer<TransformerFactory>
                    ::template dest_type<combination<kls, Elements...>>
                    ::type output_type;

            template <typename IndexTuple, typename DestType>
                struct transform_tuple_impl;

            template <int... Indexes, typename... Dest>
                struct transform_tuple_impl<index_tuple<Indexes...>, combination<kls, Dest...>> {
                    static
                    combination<kls, Dest...> &&
                    transform(combination<kls, Elements...> && comb)
                    {
                        /*return { std::forward<typename dest_type<Elements>::type>(*/
                        /*return { std::forward(*/
                        return std::move(combination<kls, Dest...>(
                                   static_cast<Dest &&>(
                                       transformer<TransformerFactory>::transform(
                                           std::get<Indexes>(comb)))...
                                   ));
                                                /*comb)))...) };*/
                    }

                };

            static output_type transform(const combination<kls, Elements...> & comb)
            {
                return static_cast<output_type &&>(
                        transform_tuple_impl<typename make_indexes<Elements...>::type,
                                             typename dest_type<combination<kls, Elements...>>::type>
                            ::transform(static_cast<combination<kls, Elements...> &&>(combination<kls, Elements...>(comb))));
            }

            static output_type && transform(combination<kls, Elements...> && comb)
            {
                return static_cast<output_type &&>(
                        transform_tuple_impl<typename make_indexes<Elements...>::type,
                                             typename dest_type<combination<kls, Elements...>>::type>
                            ::transform(static_cast<combination<kls, Elements...> &&>(comb)));
            }
        };

    template <typename kls, typename... Elements>
        static
        auto transform(combination<kls, Elements...> && x)
            -> typename dest_type<combination<kls, Elements...>>::type
        {
            typedef typename dest_type<combination<kls, Elements...>>::type return_type;
            return transform_tuple<kls, Elements...>::transform(x);
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
    /*typedef Elements elements;*/

    template <typename IndexTuple> struct clone_helper;
    template <int... Index>
        struct clone_helper<index_tuple<Index...>> {
            typedef std::tuple<Elements...> return_type;

            static return_type &&
            combine(const combination<kls, Elements...> & c)
            {
                return std::move(return_type(
                           Elements(std::get<Index>(c))...
                       ));
            }
        };

    combination(Elements && ... y)
        : std::tuple<Elements...>(y...)
    {}
    /*combination(const combination<kls, Elements...>& c)*/
        /*: std::tuple<Elements...>(clone_helper<typename make_indexes<Elements...>::type>::combine(c))*/
    /*{}*/
    /*combination(std::tuple<Elements...> && y) : std::tuple<Elements...>(y) {}*/

    template <typename IndexTuple, typename Element> struct combinator_helper;
    template <int... Index, typename Element>
        struct combinator_helper<index_tuple<Index...>, Element> {
            typedef combination<kls, Elements..., Element> return_type;

            static return_type &&
            combine(combination<kls, Elements...> & c, Element && e)
            {
                return std::move(return_type(
                           static_cast<Elements &&>(std::get<Index>(c))...,
                           static_cast<Element &&>(e)
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
            /*return { std::move(std::tuple_cat(*/
                        /*std::forward<std::tuple<Elements...>>(*this),*/
                        /*std::make_tuple(e))) };*/
            /*return { std::tuple_cat(*this, std::make_tuple(e)) };*/
        }

    template <typename Element>
        combination<kls, Elements..., Element> combine(const Element & e)
        {
            return combinator_helper<
                       typename make_indexes<Elements...>::type,
                       Element
                   >::combine(*this, Element(e));
            /*return { std::move(std::tuple_cat(*this, std::make_tuple(std::forward(e)))) };*/
            /*return { std::tuple_cat(*this, std::make_tuple(e)) };*/
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
        typename transformer<TransformerFactory>::template dest_type<combination<kls, Elements...>>::type
        transform()
        {
            /*return *this;*/
            return transformer<TransformerFactory>::transform(*this);
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

