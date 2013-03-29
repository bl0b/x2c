/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef _X2C_XML_BASE_H_
#define _X2C_XML_BASE_H_

#include "structure.h"

#define CHARDATA_NAME "#CHARDATA"

namespace x2c {

struct ignore_entity {};
struct ignore { ignore_entity entity; };

template <typename X>
using data_validator = std::function<bool(X*)>;

template <typename EvalType> struct Element;
template <typename ParentType, typename OutputType, typename EntityType> struct data_binder;
template <typename OutputType, typename kls, typename... Elements> struct iterator;
template <typename EvalType> struct xml_context;

template <typename ValueType>
struct unconst_value_type { typedef ValueType type; };
template <typename K, typename V>
struct unconst_value_type<std::pair<const K, V>> { typedef std::pair<K, V> type; };

template <typename Coll>
using mutable_value_type = typename unconst_value_type<typename Coll::value_type>::type;

template <typename EvalType>
struct Entity {
    typedef EvalType eval_type;

    std::string name;
    Entity(const std::string& n) : name(n) {}
    Entity(const Entity<EvalType>& e) : name(e.name) {}
};


struct CharData {
    typedef std::string entity_type;
    typedef void eval_type;
};

struct attr_eval {
    typedef std::string entity_type;
    typedef void eval_type;
    std::string name;
    attr_eval(const std::string& s) : name(s) {}
};


template <typename EvalType, typename Derived>
struct with_validation {
    data_validator<EvalType> validate;

    with_validation(data_validator<EvalType> pred)
    {
        validate = pred;
        std::cerr << typeid(Derived).name() << " val=" << ((bool)validate) << std::endl;
    }

    with_validation(const with_validation<EvalType, Derived>& wv)
    {
        validate = wv.validate;
    }

    template <typename Predicate>
        Derived&
        operator /= (Predicate pred)
        {
            data_validator<EvalType> previous = validate;
            validate = [previous, pred] (EvalType* x) { return previous(x) && pred(x); };
            return *static_cast<Derived*>(this);
        }

    template <typename Predicate>
        Derived&
        operator / (Predicate pred)
        {
            Derived ret(*static_cast<Derived*>(this));
            return (ret /= pred);
        }
};


template <typename StrucType, typename FieldType>
struct attr_binding : public Entity<FieldType StrucType::*>,
                      public with_validation<FieldType, attr_binding<StrucType, FieldType>> {
    using Entity<FieldType StrucType::*>::name;
    typedef with_validation<FieldType, attr_binding<StrucType, FieldType>> validation_type;

    typedef std::string entity_type;
    typedef FieldType eval_type;

    FieldType StrucType::* field;

    attr_binding(const std::string& n, FieldType StrucType::* f, data_validator<FieldType>& dv)
        : Entity<FieldType StrucType::*>(n), validation_type(dv), field(f)
    {}

    attr_binding(const attr_binding<StrucType, FieldType>& ab)
        : Entity<FieldType StrucType::*>(ab)
        , validation_type(ab)
        , field(ab.field)
    {}
};


template <>
struct attr_binding<ignore, std::string> : public with_validation<std::string, attr_binding<ignore, std::string>> {
    typedef std::string entity_type;
    typedef std::string eval_type;
    typedef with_validation<std::string, attr_binding<ignore, std::string>> validation_type;

    std::string name;

    attr_binding(const std::string& n, data_validator<std::string>& dv)
        : validation_type(dv), name(n)
    {}

    attr_binding(const attr_binding<ignore, std::string>& ab)
        : validation_type(ab), name(ab.name)
    {}
};


template <typename StrucType, typename FieldType>
struct elt_binding : public with_validation<FieldType, elt_binding<StrucType, FieldType>> {
    typedef Element<FieldType> entity_type;
    typedef FieldType StrucType::* eval_type;
    typedef with_validation<FieldType, elt_binding<StrucType, FieldType>> validation_type;

    const entity_type* elt;
    eval_type field;

    elt_binding(const Element<FieldType>* e, eval_type f, data_validator<FieldType>& dv)
        : validation_type(dv), elt(e), field(f)
    {}

    elt_binding(const elt_binding<StrucType, FieldType>& eb)
        : validation_type(eb), elt(eb.elt), field(eb.field)
    {}
};


template <typename FieldType>
struct elt_binding<ignore, FieldType> : public with_validation<FieldType, elt_binding<ignore, FieldType>> {
    typedef Element<FieldType> entity_type;
    typedef FieldType eval_type;
    typedef with_validation<FieldType, elt_binding<ignore, FieldType>> validation_type;

    const entity_type* elt;

    elt_binding(const Element<FieldType>* e, data_validator<FieldType>& dv)
        : validation_type(dv), elt(e)
    {}

    elt_binding(const elt_binding<ignore, FieldType>& e)
        : validation_type(e), elt(e.elt)
    {}
};

template <typename StrucType, typename CollType>
struct elt_coll_binding : public with_validation<mutable_value_type<CollType>, elt_coll_binding<StrucType, CollType>> {
    typedef CollType collection_type;
    typedef mutable_value_type<CollType> value_type;
    typedef Element<value_type> entity_type;
    typedef with_validation<mutable_value_type<CollType>, elt_coll_binding<StrucType, CollType>> validation_type;

    const entity_type* elt;
    collection_type StrucType::* field;

    elt_coll_binding(const entity_type* e, collection_type StrucType::* f, data_validator<value_type> dv)
        : validation_type(dv), elt(e), field(f)
    {}

    elt_coll_binding(const elt_coll_binding<StrucType, CollType>& ecb)
        : validation_type(ecb), elt(ecb.elt), field(ecb.field)
    {}
};

template <typename StrucType, typename FieldType>
struct elt_alloc_binding : public with_validation<FieldType, elt_alloc_binding<StrucType, FieldType>> {
    typedef Element<FieldType> entity_type;
    typedef FieldType* StrucType::* eval_type;
    typedef with_validation<FieldType, elt_alloc_binding<StrucType, FieldType>> validation_type;

    const entity_type* elt;
    eval_type field;

    elt_alloc_binding(const Element<FieldType>* e, eval_type f, data_validator<FieldType> dv)
        : validation_type(dv), elt(e), field(f)
    {}

    elt_alloc_binding(const elt_alloc_binding<StrucType, FieldType>& eab)
        : validation_type(eab), elt(eab.elt), field(eab.field)
    {}
};


template <typename OutputType>
static void from_string(const std::string& s, OutputType& o)
{
    std::stringstream(s) >> o;
}


#define HAS_MEM_FUNC(func, name)                                        \
    template<typename T, typename Sign>                                 \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func> *); \
        template <typename   > static no  &chk(...);                    \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }

HAS_MEM_FUNC(push_back, has_push_back);

template <typename T>
    struct select_insertion_method {
        struct with_insert {
            template <typename C>
                static void add(C& c, const typename C::value_type& v)
                { c.insert(v); }
        };

        struct with_push_back {
            template <typename C>
                static void add(C& c, const typename C::value_type& v)
                { c.push_back(v); }
        };

        typedef typename std::conditional<
            has_push_back<T, void (T::*) (const typename T::value_type&)>::value,
            with_push_back,
            with_insert>::type _;

        static void add(T& container, const typename T::value_type& v)
        {
            _::add(container, v);
        }
    };


template<typename T>
    struct is_container {
    private:
        template<typename C> static char test(typename C::value_type*);
        template<typename C> static int  test(...);
    public:
        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };

}

#endif

