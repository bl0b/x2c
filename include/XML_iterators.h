#ifndef _XML_ITERATORS_H_
#define _XML_ITERATORS_H_

#include "XML_base.h"
#include <vector>


template <typename A, typename B, typename C>
std::string get_name(const data_binder<A, B, C>& d) { return d.name; }

template <typename kls, typename A, typename B, typename C>
std::string get_name(const combination<kls, data_binder<A, B, C>>& c) { return std::get<0>(c).name; }

template <typename OutputType>
struct iterator_base {
    bool state : 1;
    bool next : 1;

    iterator_base(bool s) : state(s), next(true) {}
    virtual ~iterator_base() {}

    virtual bool accept(const std::string& name) = 0;
    virtual bool consume(const std::string& name, xml_context<OutputType>* context) = 0;

    bool is_good() const { return state; }
    bool is_accepting() const { return next; }
    virtual bool is_done() const { return state && !next; }
};


template <typename OutputType>
struct iterator_empty : public iterator_base<OutputType> {
    using iterator_base<OutputType>::next;

    iterator_empty() : iterator_base<OutputType>(true) { next = false; }
    bool accept(const std::string& name) { return false; (void)name; }
    bool consume(const std::string& name, xml_context<OutputType>* context) { return false; (void)name; (void)context; }
};


template <typename OutputType, typename kls, typename... Elements>
iterator<OutputType, kls, Elements...>* make_iterator(const combination<kls, Elements...>& comb);


template <typename OutputType, typename SubOutputType, typename EntityType>
struct iterator<OutputType, single, data_binder<OutputType, SubOutputType, EntityType>> : public iterator_base<OutputType> {
    using iterator_base<OutputType>::state;
    using iterator_base<OutputType>::next;

    data_binder<OutputType, SubOutputType, EntityType> binder;

    iterator(const combination<single, data_binder<OutputType, SubOutputType, EntityType>>& comb)
        : iterator_base<OutputType>(false), binder(std::get<0>(comb))
    {
        debug_log << "ITERATOR on " << binder.name << debug_endl;
    }

    bool accept(const std::string& name)
    {
        return next && name == binder.name;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        DEBUG;
        state = accept(name);
        if (state && context) {
            context->install(binder);
        }
        next = false;
        return state;
    }
};


template <typename OutputType, typename SubOutputType>
struct iterator<OutputType, single, data_binder<OutputType, SubOutputType, std::string>> : public iterator_base<OutputType> {
    using iterator_base<OutputType>::state;
    using iterator_base<OutputType>::next;

    data_binder<OutputType, SubOutputType, std::string> binder;

    iterator(const combination<single, data_binder<OutputType, SubOutputType, std::string>>& comb)
        : iterator_base<OutputType>(false), binder(std::get<0>(comb))
    {
        debug_log << "ITERATOR on " << binder.name << debug_endl;
    }

    bool accept(const std::string& name)
    {
        debug_log << "accept? " << name << " vs " << binder.name << ", next=" << next << debug_endl;
        return next && name == binder.name;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        DEBUG;
        state = accept(name);
        debug_log << "state=" << state << debug_endl;
        if (state && context) {
            context->install(binder);
        }
        next = false;
        return state;
    }
};


template <typename OutputType, typename SubOutputType, typename EntityType>
struct iterator<OutputType, multiple, data_binder<OutputType, SubOutputType, EntityType>> : public iterator_base<OutputType> {
    using iterator_base<OutputType>::state;
    using iterator_base<OutputType>::next;

    data_binder<OutputType, SubOutputType, EntityType> binder;

    iterator(const combination<multiple, data_binder<OutputType, SubOutputType, EntityType>>& comb)
        : iterator_base<OutputType>(false), binder(std::get<0>(comb))
    {
        debug_log << "ITERATOR on " << binder.name << debug_endl;
    }

    bool accept(const std::string& name)
    {
        return name == binder.name;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        state = accept(name);
        if (state && context) {
            context->install(binder);
        }
        if (state) {
            std::cerr << "[[" << typeid(binder).name() << "]]";
        } else {
            std::cerr << "[[FAILED]]";
        }
        next = state;
        return state;
    }

    bool is_done() const { return state; }
};


template <typename OutputType, typename kls, typename SubOutputType, typename EntityType>
struct iterator<OutputType, optional<kls>, data_binder<OutputType, SubOutputType, EntityType>>
: public iterator<OutputType, kls, data_binder<OutputType, SubOutputType, EntityType>> {
    using iterator<OutputType, kls, data_binder<OutputType, SubOutputType, EntityType>>::state;

    iterator(const combination<optional<kls>, data_binder<OutputType, SubOutputType, EntityType>>& ent)
        : iterator<OutputType, kls, data_binder<OutputType, SubOutputType, EntityType>>(ent)
    { state = true; }

    bool is_done() const { return true; }
};



namespace detail {
    struct init_next_any {
        bool ret;
        template <typename Iterator>
            void operator () (Iterator& i) { ret |= i.next; }
    };

    struct accept_any {
        const std::string& name;
        bool ret;
        template <typename Iterator>
            void operator () (Iterator& i) { ret |= i.accept(name); }
    };

    template <typename OutputType>
    struct consume_first {
        const std::string name;
        bool state;
        bool next;
        xml_context<OutputType>* context;
        template <typename kls, typename... Elements>
            void operator () (iterator<OutputType, kls, Elements...>& i)
            {
                if (!state) {
                    state = i.consume(name, context);
                    next = i.next;
                }
            }
    };

    struct is_done_all {
        bool ret;
        template <typename Iterator>
            void operator () (const Iterator& i) { ret &= i.is_done(); }
    };

    struct is_done_any {
        bool ret;
        template <typename Iterator>
            void operator () (const Iterator& i) { ret |= i.is_done(); }
    };

    struct accept_current {
        const std::string& name;
        size_t current;
        bool ret;
        template <typename Iterator>
            void operator () (size_t index, Iterator& i)
            {
                if (index == current) {
                    ret = i.accept(name);
                }
            }
    };

    template <typename OutputType>
    struct consume_current {
        const std::string name;
        size_t current;
        size_t last;
        bool state;
        bool next;
        xml_context<OutputType>* context;
        template <typename kls, typename... Elements>
            void operator () (size_t index, iterator<OutputType, kls, Elements...>& i)
            {
                if (index == current) {
                    if (i.accept(name)) {
                        state = i.consume(name, context);
                        next = i.next;
                    } else if (i.is_done() && current < last) {
                        ++current;
                    } else {
                        state = next = 0;
                    }
                } else if (index > current) {
                    next |= i.is_accepting();
                }
            }
    };

    struct is_done_current {
        size_t current;
        bool ret;
        template <typename Iterator>
            void operator () (size_t index, const Iterator& i)
            {
                if (current <= index) {
                    ret &= i.is_done();
                }
            }
    };
}

template <typename OutputType, typename kls, typename... Elements>
struct iterator_collection : public iterator_base<OutputType> {
    using iterator_base<OutputType>::state;
    using iterator_base<OutputType>::next;

    struct factory {
        template <typename... AnyElements>
            static
            iterator<OutputType, unordered_sequence, AnyElements...>
            transform(const combination<unordered_sequence, AnyElements...>& comb)
            {
                return { comb };
            }
        template <typename... AnyElements>
            static
            iterator<OutputType, ordered_sequence, AnyElements...>
            transform(const combination<ordered_sequence, AnyElements...>& comb)
            {
                return { comb };
            }
        template <typename... AnyElements>
            static
            iterator<OutputType, alternative, AnyElements...>
            transform(const combination<alternative, AnyElements...>& comb)
            {
                return { comb };
            }
        template <typename AnyOutputType, typename AnyKls, typename AnyEntityType>
            static
            iterator<OutputType, AnyKls, data_binder<OutputType, AnyOutputType, AnyEntityType>>
            transform(const combination<AnyKls, data_binder<OutputType, AnyOutputType, AnyEntityType>>& comb)
            {
                return { comb };
            }
    };

    template <typename Element>
    struct helper {
        static const Element& widget();
        typedef decltype(factory::transform(widget())) type;
    };

    typedef std::tuple<typename helper<Elements>::type...> contents_type;
    
    contents_type contents;

    iterator_collection(const combination<kls, Elements...>& comb)
        : iterator_base<OutputType>(false)
        , contents(map_tuple<factory, contents_type, Elements...>::transform(comb))
    {}

    void update_next()
    {
        detail::init_next_any any { false };
        iterate_over_tuple(any, contents);
        next = any.ret;
    }
};


template <typename OutputType, typename... Elements>
struct iterator<OutputType, unordered_sequence, Elements...>
    : public iterator_collection<OutputType, unordered_sequence, Elements...>
{
    using iterator_collection<OutputType, unordered_sequence, Elements...>::contents;
    using iterator_collection<OutputType, unordered_sequence, Elements...>::state;
    using iterator_collection<OutputType, unordered_sequence, Elements...>::next;
    using iterator_collection<OutputType, unordered_sequence, Elements...>::update_next;

    iterator(const combination<unordered_sequence, Elements...>& comb)
        : iterator_collection<OutputType, unordered_sequence, Elements...>(comb)
    {
        update_next();
    }

    bool accept(const std::string& name)
    {
        detail::accept_any any { name, false };
        iterate_over_tuple(any, contents);
        return any.ret;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        detail::consume_first<OutputType> first { name, false, false, context };
        iterate_over_tuple(first, contents);
        state = first.state;
        next = first.next;
        return state;
    }

    bool is_done() const
    {
        detail::is_done_all all { true };
        iterate_over_tuple(all, contents);
        return all.ret;
    }
};


template <typename OutputType, typename... Elements>
struct iterator<OutputType, alternative, Elements...>
    : public iterator_collection<OutputType, alternative, Elements...>
{
    using iterator_collection<OutputType, alternative, Elements...>::contents;
    using iterator_collection<OutputType, alternative, Elements...>::state;
    using iterator_collection<OutputType, alternative, Elements...>::next;
    using iterator_collection<OutputType, alternative, Elements...>::update_next;

    iterator(const combination<alternative, Elements...>& comb)
        : iterator_collection<OutputType, alternative, Elements...>(comb)
    {
        update_next();
    }

    bool accept(const std::string& name)
    {
        detail::accept_any any { name, false };
        iterate_over_tuple(any, contents);
        update_next();
        return any.ret;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        detail::consume_first<OutputType> first { name, false, false, context };
        iterate_over_tuple(first, contents);
        state = first.state;
        update_next();
        return state;
    }

    bool is_done() const
    {
        detail::is_done_any any { false };
        iterate_over_tuple(any, contents);
        return any.ret;
    }
};


template <typename OutputType, typename... Elements>
struct iterator<OutputType, ordered_sequence, Elements...>
    : public iterator_collection<OutputType, ordered_sequence, Elements...>
{
    using iterator_collection<OutputType, ordered_sequence, Elements...>::contents;
    using iterator_collection<OutputType, ordered_sequence, Elements...>::state;
    using iterator_collection<OutputType, ordered_sequence, Elements...>::next;

    size_t current;

    iterator(const combination<ordered_sequence, Elements...>& comb)
        : iterator_collection<OutputType, ordered_sequence, Elements...>(comb)
        , current(0)
    {}

    bool accept(const std::string& name)
    {
        detail::accept_current cur { name, current, false };
        iterate_over_tuple_with_index(cur, contents);
        return cur.ret;
    }

    bool consume(const std::string& name, xml_context<OutputType>* context)
    {
        detail::consume_current<OutputType> cur { name, current, std::tuple_size<decltype(contents)>::value - 1, false, false, context };
        iterate_over_tuple_with_index(cur, contents);
        current = cur.current;
        state = cur.state;
        next = cur.next;
        std::cerr << '<' << current << ' ' << state << ' ' << next << '>' << std::endl;
        return state;
    }

    bool is_done() const
    {
        detail::is_done_current all { current, true };
        iterate_over_tuple_with_index(all, contents);
        return all.ret;
    }
};


template <typename OutputType, typename kls, typename... Elements>
iterator<OutputType, kls, Elements...>* make_iterator(const combination<kls, Elements...>& comb)
{
    return new iterator<OutputType, kls, Elements...>(comb);
}

#endif

