#ifndef _XML_ITERATORS_H_
#define _XML_ITERATORS_H_

#include "XML_base.h"
#include <vector>

template <typename StrucType, typename FieldType, typename Structure>
std::string get_name(const attr_eval& c)
{
    return c.name;
}

template <typename StrucType, typename FieldType>
std::string get_name(const attr_bound<StrucType, FieldType>& c)
{
    return c.name;
}

template <typename StrucType, typename FieldType, typename Structure>
std::string get_name(const std::pair<const Element<FieldType, Structure>*, FieldType StrucType::*>& c)
{
    return c.first->name;
}

template <typename EvalType, typename Structure>
std::string get_name(const Element<EvalType, Structure>* c)
{
    return c->name;
}

template <typename kls, typename Elt>
std::string get_name(const combination<kls, Elt>& c) { return get_name(std::get<0>(c)); }

struct iterator_base {
    bool state : 1;
    bool next : 1;

    iterator_base(bool s) : state(s), next(true) {}

    virtual bool accept(const std::string& name) = 0;
    virtual bool consume(const std::string& name) = 0;

    bool is_good() const { return state; }
    bool is_accepting() const { return next; }
    virtual bool is_done() const { return state && !next; }
};

template <typename kls>
struct iterator;

template <typename kls, typename... Elements>
iterator<kls>* make_iterator(const combination<kls, Elements...>& comb);

template <>
struct iterator<single> : public iterator_base {
    std::string ent_name;

    template <typename Ent>
    iterator(const combination<single, Ent>& ent)
        : iterator_base(false), ent_name(get_name(ent))
    {}

    bool accept(const std::string& name)
    {
        return next && name == ent_name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = false;
        return state;
    }
};

template <>
struct iterator<multiple> : public iterator_base {
    std::string ent_name;

    template <typename Ent>
    iterator(const combination<multiple, Ent>& ent)
        : iterator_base(false), ent_name(get_name(ent))
    {}

    bool accept(const std::string& name)
    {
        return name == ent_name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = state;
        return state;
    }

    bool is_done() const { return state; }
};

template <>
struct iterator<optional<single>> : public iterator_base {
    std::string ent_name;

    template <typename Ent>
    iterator(const combination<optional<single>, Ent>& ent)
        : iterator_base(true), ent_name(get_name(ent))
    {}

    bool accept(const std::string& name)
    {
        return next && name == ent_name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = false;
        return state;
    }

    bool is_done() const { return true; }
};

template <>
struct iterator<optional<multiple>> : public iterator_base {
    std::string ent_name;

    template <typename Ent>
    iterator(const combination<optional<multiple>, Ent>& ent)
        : iterator_base(true), ent_name(get_name(ent))
    {}

    bool accept(const std::string& name)
    {
        return name == ent_name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = state;
        return state;
    }

    bool is_done() const { return true; }
};


struct iterator_collection : public iterator_base {
    typedef std::vector<iterator_base*> collection_type;
    collection_type sub;
    typedef collection_type::iterator coll_iterator;
    typedef collection_type::const_iterator coll_const_iterator;

    struct init_coll {
        collection_type sub;
        template <typename Combination>
        init_coll(const Combination& c)
            : sub()
        {
            iterate_over_tuple(*this, c);
        }
        template <typename Combination>
        void operator () (const Combination& c)
        {
            /*sub.push_back(new iterator<typename Combination::derived>(c));*/
            sub.push_back(make_iterator(c));
        }
    };

    template <typename... Elements, typename kls>
        iterator_collection(const combination<kls, Elements...>& comb)
            : iterator_base(false), sub(init_coll(comb).sub)
        {}

    virtual ~iterator_collection()
    {
        coll_iterator i = sub.begin(), j = sub.end();
        for (; i != j; ++i) {
            delete *i;
        }
    }
};


template <>
struct iterator<unordered_sequence> : public iterator_collection {
    using iterator_collection::coll_iterator;
    using iterator_collection::coll_const_iterator;

    template <typename... Elements, typename kls>
        iterator(const combination<kls, Elements...>& comb)
            : iterator_collection(comb)
        {
            coll_iterator i = sub.begin(), j = sub.end();
            state = false;
            next = false;
            for (; i != j; ++i) {
                state |= (*i)->state;
                next |= (*i)->next;
            }
        }

    bool accept(const std::string& name)
    {
        bool ret = false;
        coll_const_iterator i = sub.cbegin(), j = sub.cend();
        for (; i != j; ++i) {
            ret |= (*i)->accept(name);
        }
        return ret;
    }

    bool consume(const std::string& name)
    {
        coll_iterator i = sub.begin(), j = sub.end();
        for (; i != j && !(*i)->accept(name); ++i);
        if (i != j) {
            state = (*i)->consume(name);
            next = (*i)->next;
        }
        return state;
    }

    bool is_done() const
    {
        bool ret = true;
        coll_const_iterator i = sub.cbegin(), j = sub.cend();
        for (; i != j; ++i) {
            ret &= (*i)->is_done();
        }
        return ret;
    }
};


template <>
struct iterator<ordered_sequence> : public iterator_collection {
    using iterator_collection::coll_iterator;
    using iterator_collection::coll_const_iterator;

    coll_iterator cursor;

    template <typename... Elements, typename kls>
        iterator(const combination<kls, Elements...>& comb)
            : iterator_collection(comb)
        {
            cursor = sub.begin();
            state = (*cursor)->state;
            next = (*cursor)->next;
        }

    bool is_done() const
    {
        if (cursor == sub.end()) {
            return true;
        }
        coll_iterator succ = cursor;
        do {
            ++succ;
        } while (succ != sub.end() && (*succ)->is_done());
        return (*cursor)->is_done() && succ == sub.end();
    }

    bool accept(const std::string& name)
    {
        while(cursor != sub.end() && !(*cursor)->accept(name) && (*cursor)->is_done()) {
            ++cursor;
        }
        return cursor != sub.end() && (*cursor)->accept(name);
    }

    bool consume(const std::string& name)
    {
        while (cursor != sub.end() && !(*cursor)->accept(name) && (*cursor)->is_done()) {
            ++cursor;
        }
        if (cursor == sub.end()) {
            state = false;
            next = false;
        } else {
            state = (*cursor)->consume(name);
            coll_iterator succ = cursor;
            ++succ;
            next = (*cursor)->next || succ != sub.end();
        }
        return state;
    }
};


template <>
struct iterator<alternative> : public iterator_collection {
    using iterator_collection::coll_iterator;
    using iterator_collection::coll_const_iterator;

    template <typename... Elements, typename kls>
        iterator(const combination<kls, Elements...>& comb)
            : iterator_collection(comb)
        {
            coll_iterator i = sub.begin(), j = sub.end();
            state = false;
            next = false;
            for (; i != j; ++i) {
                state |= (*i)->state;
                next |= (*i)->next;
            }
        }

    bool accept(const std::string& name)
    {
        bool ret = false;
        coll_iterator i = sub.begin(), j = sub.end();
        for (; i != j;) {
            if ((*i)->accept(name)) {
                ret = true;
                ++i;
            } else {
                delete *i;
                i = sub.erase(i);
            }
        }
        return ret;
    }

    bool consume(const std::string& name)
    {
        state = false;
        coll_iterator i = sub.begin(), j = sub.end();
        for (; i != j; ++i) {
            state |= (*i)->consume(name);
        }
        return state;
    }

    bool is_done() const
    {
        bool ret = false;
        coll_const_iterator i = sub.cbegin(), j = sub.cend();
        for (; i != j; ++i) {
            ret |= (*i)->is_done();
        }
        return ret;
    }
};



template <typename kls, typename... Elements>
iterator<kls>* make_iterator(const combination<kls, Elements...>& comb)
{
    return new iterator<kls>(comb);
}

#endif

