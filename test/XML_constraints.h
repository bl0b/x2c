#ifndef _XML_CONSTRAINTS_H_
#define _XML_CONSTRAINTS_H_

#include <vector>

template <typename Struc, typename FieldType>
struct target_descr {
    typedef Struc struct_type;
    typedef FieldType field_type;
    std::string name;
    FieldType Struc::* field;
    target_descr(const char* n, FieldType Struc::* f) : name(n), field(f) {
        std::cout << name << std::endl;
    }
    target_descr(const target_descr& t) : name(t.name), field(t.field) {
        std::cout << name << std::endl;
    }
};

template <typename Struc, typename FieldType>
struct attribute_target_descr : public target_descr<Struc, FieldType> {
    attribute_target_descr(const char* n, FieldType Struc::* f)
        : target_descr<Struc, FieldType>(n, f)
    {}
};
template <typename Struc, typename FieldType>
struct element_target_descr : public target_descr<Struc, FieldType> {
    element_target_descr(const char* n, FieldType Struc::* f)
        : target_descr<Struc, FieldType>(n, f)
    {}
};


template <typename Struc, typename FieldType>
combination<single, attribute_target_descr<Struc, FieldType>> &&
A(const char* name, FieldType Struc::* field)
{
    return make_single(attribute_target_descr<Struc, FieldType>(name, field));
}

template <typename Struc, typename FieldType>
combination<single, element_target_descr<Struc, FieldType>> &&
E(const char* name, FieldType Struc::* field)
{
    return make_single(element_target_descr<Struc, FieldType>(name, field));
}

template <typename Struc, typename FieldType>
combination<multiple, element_target_descr<Struc, std::vector<FieldType>>> &&
E(const char* name, std::vector<FieldType> Struc::* field)
{
    return make_multiple(element_target_descr<Struc, std::vector<FieldType>>(name, field));
}


#endif

