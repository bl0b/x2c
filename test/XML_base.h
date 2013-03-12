#ifndef _XML_BASE_H_
#define _XML_BASE_H_

#include "structure.h"

template <typename EvalType>
struct Entity {
    typedef EvalType eval_type;

    std::string name;
    Entity(const std::string& n) : name(n) {}
    Entity(std::string && n) : name(n) {}
};


struct CharData {};

struct attr_eval {
    typedef void eval_type;
    std::string name;
    attr_eval(const std::string& s) : name(s) {}
};


template <typename StrucType, typename FieldType>
struct attr_bound : public Entity<FieldType StrucType::*> {
    using Entity<FieldType StrucType::*>::name;
    using typename Entity<FieldType StrucType::*>::eval_type;

    eval_type field;

    attr_bound(const std::string& n, eval_type f) : Entity<FieldType StrucType::*>(n), field(f) {}
    attr_bound(std::string && n, eval_type f) : Entity<FieldType StrucType::*>(n), field(f) {}
};


template <typename OutputType>
struct from_string {
    static void convert(const std::string& s, OutputType& o)
    {
        std::stringstream(s) >> o;
    }
};

template <typename EvalType, typename Structure> struct Element;
template <typename OutputType, typename EntityType> struct data_binder;

#endif

