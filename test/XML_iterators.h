#ifndef _XML_ITERATORS_H_
#define _XML_ITERATORS_H_

#include "XML_base.h"
#include <vector>


template <typename A, typename B>
std::string get_name(const data_binder<A, B>& d) { return d.name; }

template <typename kls, typename A, typename B>
std::string get_name(const combination<kls, data_binder<A, B>>& c) { return std::get<0>(c).name; }

struct iterator_base {
    bool state : 1;
    bool next : 1;

    iterator_base(bool s) : state(s), next(true) {}
    virtual ~iterator_base() {}

    virtual bool accept(const std::string& name) = 0;
    virtual bool consume(const std::string& name) = 0;

    bool is_good() const { return state; }
    bool is_accepting() const { return next; }
    virtual bool is_done() const { return state && !next; }
};

template <typename OutputType, typename kls, typename... Elements>
struct iterator;

template <typename OutputType, typename kls, typename... Elements>
iterator<OutputType, kls, typename Elements...>* make_iterator(const combination<kls, Elements...>& comb);

template <typename OutputType, typename OutputType, typename EntityType>
struct iterator<OutputType, single, data_binder<OutputType, EntityType>> : public iterator_base {
    EntityType* ent;

    iterator(const combination<single, EntityType>& comb)
        : iterator_base(false), ent(&std::get<0>(c))
    {}

    bool accept(const std::string& name)
    {
        return next && name == ent->name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = false;
        return state;
    }
};

template <typename OutputType, typename EntityType>
struct iterator<OutputType, multiple, data_binder<OutputType, EntityType>> : public iterator_base {
    EntityType* ent;

    iterator(const combination<multiple, EntityType>& comb)
        : iterator_base(false), ent(&std::get<0>(c))
    {}

    bool accept(const std::string& name)
    {
        return name == ent->name;
    }

    bool consume(const std::string& name)
    {
        state = accept(name);
        next = state;
        return state;
    }

    bool is_done() const { return state; }
};

template <typename OutputType, typename EntityType>
struct iterator<OutputType, optional<single>, data_binder<EvalType, EntityType>> : public iterator<OutputType, single, data_binder<OutputType, EntityType>> {
    iterator(const combination<optional<single>, EntityType>& ent)
        : iterator<single, data_binder<EvalType, EntityType>>(ent)
    {}

    bool is_done() const { return true; }
};

template <typename OutputType, typename EntityType>
struct iterator<OutputType, optional<multiple>, data_binder<EvalType, EntityType>> : public iterator<OutputType, multiple, data_binder<OutputType, EntityType>> {
    iterator(const combination<optional<multiple>, EntityType>& ent)
        : iterator<multiple, data_binder<EvalType, EntityType>>(ent)
    {}

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


template <typename... Elements>
struct iterator<unordered_sequence, Elements...> : public iterator_collection {
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

