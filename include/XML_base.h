#ifndef _XML_BASE_H_
#define _XML_BASE_H_

#include "structure.h"

#define CHARDATA_NAME "#CHARDATA"

template <typename EvalType> struct Element;
template <typename ParentType, typename OutputType, typename EntityType> struct data_binder;
template <typename OutputType, typename kls, typename... Elements> struct iterator;
template <typename EvalType> struct xml_context;

template <typename EvalType>
struct Entity {
    typedef EvalType eval_type;

    std::string name;
    Entity(const std::string& n) : name(n) {}
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


template <typename StrucType, typename FieldType>
struct attr_binding : public Entity<FieldType StrucType::*> {
    using Entity<FieldType StrucType::*>::name;

    typedef std::string entity_type;
    typedef FieldType eval_type;

    FieldType StrucType::* field;

    attr_binding(const std::string& n, FieldType StrucType::* f) : Entity<FieldType StrucType::*>(n), field(f) {}
    attr_binding(std::string && n, FieldType StrucType::* f) : Entity<FieldType StrucType::*>(n), field(f) {}
};


template <typename StrucType, typename FieldType>
struct elt_binding {
    typedef Element<FieldType> entity_type;
    typedef FieldType StrucType::* eval_type;

    const entity_type* elt;
    eval_type field;
};

template <typename StrucType, typename CollType>
struct elt_coll_binding {
    typedef CollType collection_type;
    typedef typename collection_type::value_type value_type;
    typedef Element<value_type> entity_type;

    const entity_type* elt;
    collection_type StrucType::* field;
};

template <typename StrucType, typename FieldType>
struct elt_alloc_binding {
    typedef Element<FieldType> entity_type;
    typedef FieldType* StrucType::* eval_type;

    const entity_type* elt;
    eval_type field;
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

#endif

