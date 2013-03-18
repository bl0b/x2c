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

template <typename StrucType, typename CollType, typename Function>
struct elt_coll_binding {
    typedef CollType collection_type;
    typedef typename collection_type::value_type value_type;
    typedef Element<value_type> entity_type;

    const entity_type* elt;
    collection_type StrucType::* field;
    Function collection_type::* insert;
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



template<typename T>
    struct is_container {
    private:
        template<typename C> static char test(typename C::value_type*);
        template<typename C> static int  test(...);
    public:
        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };

#endif

