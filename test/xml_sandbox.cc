#include "XML_base.h"

#include <expat.h>

template <typename EvalType>
struct xml_parser {
};




#include "XML_iterators.h"
#include "XML_databinding.h"





resolve_bindings<int> test;
struct check {};
resolve_bindings<check> test_compound;


template <typename EvalType>
struct Element : public Entity<EvalType> {
    using Entity<EvalType>::eval_type;
    using Entity<EvalType>::name;
    typedef transformer<resolve_bindings<EvalType>> linker;

    std::function<iterator_base*()> iterator;

    Element(const char* n)
        : Entity<EvalType>(n)
    {
        iterator = [] () { return nullptr; };
    }

    template <typename Structure>
        Element(const std::string & n, const Structure & s)
            : Entity<EvalType>(n)
        {
            make_gen_iterator(s);
        }

    template <typename Structure>
    Element<EvalType> operator || (const Structure& s)
    {
        return Element<EvalType>(name, s);
    }

    template <typename Structure>
    Element<EvalType>& operator = (Structure && s)
    {
        return make_gen_iterator(std::move(s));
    }

    template <typename Structure>
        Element<EvalType>& make_gen_iterator(Structure && s)
        {
            auto transformed = linker::transform(std::move(s));
            iterator = [=] () {
                return make_iterator<EvalType>(transformed);
            };
            return *this;
        }
};





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


#if 0
struct dtd {
    Element<int> a;
    Element<double> b;
    Element<double> factor;
    Element<Manip> op;
    Element<Foo> foo;

    dtd()
        : a("a")
        , b("b")
        , factor("factor")
        , op("op")
        , foo("foo")
    {
        DEBUG;
        a = chardata();
        b = A("value");
        factor = chardata();
        op = E(factor, &Manip::factor);
        foo = E(a, &Foo::a) & E(b, &Foo::b) & make_optional(E(op));
        /*foo = E(a, &Foo::a) & E(b, &Foo::b) & E(op);*/
    }
};
#endif


#define DTD_START(dtd_name, root_name, eval_type) \
    struct dtd_name ## _type { \
        Element<eval_type> root_name; \
        dtd_name ## _type () \
            : root_name(#root_name) \
        {

#define ELEMENT(elt_name, eval_type) Element<eval_type> elt_name(#elt_name)

#define DTD_END(dtd_name) \
        } \
    } dtd_name;

/*DTD_START(dtd, foo, Foo)*/
DTD_START(dtd, op, Manip)
    ELEMENT(a, int);
    ELEMENT(b, double);
    ELEMENT(factor, double);
    /*ELEMENT(op, Manip);*/
    ELEMENT(foo, Foo);

    a = chardata();
    b = A("value");
    factor = chardata();
    op = OE(factor, &Manip::factor);
    foo = E(a, &Foo::a) & E(b, &Foo::b) & E(op);
DTD_END(dtd)


const char* XML1 = "<foo><a>1234</a><b value=\"43.21\"></foo>";
const char* XML2 = "<foo><a>1234</a><b value=\"43.21\"><op><factor>100</factor></op></foo>";

std::ostream& operator << (std::ostream& o, iterator_base& i)
{
    return o << "<s=" << i.state << " n=" << i.next << "  g?=" << i.is_good() << " a?=" <<  i.is_accepting() << " d?=" << i.is_done() << '>';
}

void feed(std::string iter_name, std::string name, iterator_base& i)
{
    std::cout << iter_name << " << " << name << ' ';
    i.consume(name);
    std::cout << i << std::endl;
}

int main(int argc, char** argv)
{
#if 1
    auto iop = dtd.op.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    std::cout << "iop " << (*iop) << std::endl;
    feed("iop", "factor", *iop);
    feed("iop", "factor", *iop);
    std::cout << std::endl;
#else
    auto ifoo = dtd.foo.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    std::cout << "ifoo " << (*ifoo) << std::endl;
    feed("ifoo", "a", *ifoo);
    feed("ifoo", "b", *ifoo);
    feed("ifoo", "op", *ifoo);
    feed("ifoo", "op", *ifoo);
#endif
    return 0;
}

