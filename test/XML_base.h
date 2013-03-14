#ifndef _XML_BASE_H_
#define _XML_BASE_H_

#include "structure.h"

template <typename EvalType> struct Element;
template <typename OutputType, typename EntityType> struct data_binder;
template <typename EvalType> struct xml_parser;

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

    eval_type field;

    attr_binding(const std::string& n, eval_type f) : Entity<FieldType StrucType::*>(n), field(f) {}
    attr_binding(std::string && n, eval_type f) : Entity<FieldType StrucType::*>(n), field(f) {}
};


template <typename StrucType, typename FieldType>
struct elt_binding {
    typedef Element<FieldType> entity_type;
    typedef FieldType StrucType::* eval_type;

    const entity_type* elt;
    eval_type field;
};


template <typename OutputType>
static void from_string(const std::string& s, OutputType& o)
{
    std::stringstream(s) >> o;
}

#endif

