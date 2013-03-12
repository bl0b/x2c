#ifndef _XML_CONSTRAINTS_H_
#define _XML_CONSTRAINTS_H_

#include <vector>

#include <cstddef>

typedef ptrdiff_t name_t;



template <typename Struc, typename FieldType>
struct target_descr {
    typedef Struc struct_type;
    typedef FieldType field_type;
    name_t name;
    FieldType Struc::* field;
    target_descr(name_t n, FieldType Struc::* f) : name(n), field(f) {
        std::cout << name << std::endl;
    }
    target_descr(const target_descr& t) : name(t.name), field(t.field) {
        std::cout << name << std::endl;
    }
};

template <typename Struc, typename FieldType>
struct attribute_target_descr : public target_descr<Struc, FieldType> {
    attribute_target_descr(name_t n, FieldType Struc::* f)
        : target_descr<Struc, FieldType>(n, f)
    {}
};
template <typename Struc, typename FieldType>
struct element_target_descr : public target_descr<Struc, FieldType> {
    element_target_descr(name_t n, FieldType Struc::* f)
        : target_descr<Struc, FieldType>(n, f)
    {}
};


template <typename T>
struct attribute_eval_to {
    name_t name;
};

template <typename T>
struct cdata_eval_to {};


template <typename Struc, typename FieldType>
combination<single, attribute_target_descr<Struc, FieldType>>
A(name_t name, FieldType Struc::* field)
{
    return make_single(attribute_target_descr<Struc, FieldType>(name, field));
}

template <typename Struc, typename FieldType>
combination<single, element_target_descr<Struc, FieldType>>
E(name_t name, FieldType Struc::* field)
{
    return make_single(element_target_descr<Struc, FieldType>(name, field));
}

template <typename Struc, typename FieldType>
combination<multiple, element_target_descr<Struc, std::vector<FieldType>>>
E(name_t name, std::vector<FieldType> Struc::* field)
{
    return make_multiple(element_target_descr<Struc, std::vector<FieldType>>(name, field));
}



#endif

