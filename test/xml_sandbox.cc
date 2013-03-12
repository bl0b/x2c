#include "XML_base.h"
#include "XML_iterators.h"

/* simple setter */
template <typename StrucType, typename FieldType, typename Structure>
struct data_binder<FieldType StrucType::*, Element<StrucType, Structure>> {
    std::string name;
    FieldType StrucType::* target;
    typedef Element<FieldType, Structure> entity_type;

    FieldType* install(StrucType* container) const
    {
        return &container->*target;
    }

    void after(StrucType* container, FieldType data) const
    {
        container->*target = data;
    }
};


/* simple setter with allocator */
template <typename StrucType, typename FieldType, typename Structure>
struct data_binder<FieldType* StrucType::*, Element<StrucType, Structure>> {
    std::string name;
    FieldType* StrucType::* target;
    typedef Element<FieldType, Structure> entity_type;

    FieldType* install(StrucType* container) const
    {
        container->*target = new FieldType();
        return container->*target;
    }

    void after(StrucType* container, FieldType data) const
    {
        *container->*target = data;
    }
};


/* manipulator (target type is not related to element::eval_type) */
template <typename EvalType, typename Structure, typename Manipulator>
struct data_binder<Manipulator, Element<EvalType, Structure>> {
    std::string name;
    typedef Element<EvalType, Structure> entity_type;

    Manipulator* install(EvalType* container)
    {
        return new Manipulator();
    }

    void after(EvalType* container, Manipulator* data)
    {
        (*data)(*container);
        delete data;
    }
};


/* transient binding (target is forwarded) */
template <typename EvalType, typename Structure>
struct data_binder<EvalType, Element<EvalType, Structure>> {
    std::string name;
    typedef Element<EvalType, Structure> entity_type;

    EvalType* install(EvalType* container)
    {
        return container;
    }

    void after(EvalType* container, EvalType* data)
    {}
};



template <typename EvalType, typename Structure=void>
struct Element : public Entity<EvalType> {
    using Entity<EvalType>::eval_type;
    using Entity<EvalType>::name;

    Structure structure;
    Element(const std::string& n, const Structure& s)
        : Entity<EvalType>(n), structure(s)
    {}

    Element(std::string && n, Structure && s)
        : Entity<EvalType>(n), structure(s)
    {}
};


template <typename EvalType>
struct Element<EvalType, void> : public Entity<EvalType> {
    using Entity<EvalType>::eval_type;
    using Entity<EvalType>::name;

    Element(const char* n)
        : Entity<EvalType>(n)
    {}

    template <typename Structure>
    Element<EvalType, Structure> operator || (const Structure& s)
    {
        return { name, s };
    }

    template <typename Structure>
    Element<EvalType, Structure> operator || (Structure && s)
    {
        return { std::move(name), std::move(s) };
    }
};



inline
combination<single, CharData>
chardata() { return { CharData() }; }


combination<single, attr_eval>
A(const char* name)
{
    return { { name } };
}


template <typename StrucType, typename FieldType>
combination<single, attr_bound<StrucType, FieldType>>
A(const char* name, FieldType StrucType::* field)
{
    return { { name, field } };
}


template <typename StrucType, typename FieldType, typename Structure>
combination<single, std::pair<const Element<FieldType, Structure>*, FieldType StrucType::*>>
E(const Element<FieldType, Structure>& elt, FieldType StrucType::* field)
{
    return {
        { &elt, field }
    };
}


template <typename Manipulator, typename Structure>
combination<single, const Element<Manipulator, Structure>*>
E(const Element<Manipulator, Structure>& elt)
{
    return { &elt };
}



/*auto foo_has_op = [] (const std::string& value) { return value == "true" };*/



struct Foo {
    int a;
    double b;
};

struct Manip {
    double factor;
    Manip() : factor(1.) {}
    void operator () (Foo& f)
    {
        f.b *= factor;
    }
};

#if 1
auto ea     = Element<int>("a") || chardata();

auto eb     = Element<double>("b") || A("value");

auto efact  = Element<double>("factor") || chardata();

auto eop    = Element<Manip>("op") || E(efact, &Manip::factor);

auto efoo   = Element<Foo>("foo")
              /*|| ((E(ea, &Foo::a), E(eb, &Foo::b)) & make_optional(E(eop)));*/
              || (E(ea, &Foo::a) & E(eb, &Foo::b) & make_optional(E(eop)));


#endif

const char* XML1 = "<foo><a>1234</a><b value=\"43.21\"></foo>";
const char* XML2 = "<foo><a>1234</a><b value=\"43.21\"><op><factor>100</factor></op></foo>";

template <typename kls>
std::ostream& operator << (std::ostream& o, iterator<kls>& i)
{
    return o << "<s=" << i.state << " n=" << i.next << "  g?=" << i.is_good() << " a?=" <<  i.is_accepting() << " d?=" << i.is_done() << '>';
}

template <typename kls>
void feed(std::string iter_name, std::string name, iterator<kls>& i)
{
    std::cout << iter_name << " << " << name << ' ';
    i.consume(name);
    std::cout << i << std::endl;
}

int main(int argc, char** argv)
{
    auto iop = make_iterator(eop.structure);
    auto ifoo = make_iterator(efoo.structure);
    std::cout << "iop " << (*iop) << std::endl;
    feed("iop", "factor", *iop);
    feed("iop", "factor", *iop);
    std::cout << std::endl;
    std::cout << "ifoo " << (*ifoo) << std::endl;
    feed("ifoo", "a", *ifoo);
    feed("ifoo", "b", *ifoo);
    feed("ifoo", "op", *ifoo);
    feed("ifoo", "op", *ifoo);
    return 0;
}

