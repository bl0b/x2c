#include <fstream>

#include "XML.h"





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
        debug_log << "factor is " << factor << debug_endl;
        f.b *= factor;
    }
};

struct Init {
    void operator () (Foo& f)
    {
        f.a = 6106;
        f.b = 101.505;
    }
};

#if 0
struct some_dtd {
    Element<int> a;
    Element<double> b;
    Element<double> factor;
    Element<Manip> op;
    Element<Foo> foo;

    some_dtd()
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
#else

#if 1
DTD_START(dtd, foo, Foo)
    ELEMENT(a, int);
    ELEMENT(b, double);
    ELEMENT(factor, double);
    ELEMENT(op, Manip);
    ELEMENT(init, Init);
    /*ELEMENT(foo, Foo);*/

    a = chardata();
    b = A("value");
    factor = chardata();
    op = E(factor, &Manip::factor);
    foo = E(init) | (E(a, &Foo::a) & E(b, &Foo::b) & make_optional(E(op)));
DTD_END(dtd);
#endif

DTD_START(test1, i, int)
    i = chardata();
DTD_END(test1);

DTD_START(test2, i, int)
    i = A("value");
DTD_END(test2);

#endif


const char* XML1 = "<foo><a>1234</a><b value=\"43.21\"></foo>";
const char* XML2 = "<foo><a>1234</a><b value=\"43.21\"><op><factor>100</factor></op></foo>";
const char* XML3 = "<foo><init/></foo>";

template <typename OutputType>
std::ostream& operator << (std::ostream& o, iterator_base<OutputType>& i)
{
    return o << "<s=" << i.state << " n=" << i.next << "  g?=" << i.is_good() << " a?=" <<  i.is_accepting() << " d?=" << i.is_done() << '>';
}


template <typename OutputType>
void feed(std::string iter_name, std::string name, iterator_base<OutputType>& i)
{
    std::cout << iter_name << " << " << name << ' ';
    i.consume(name, NULL);
    std::cout << i << std::endl;
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;
#if 0
    some_dtd dtd;

    auto iop = dtd.op.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    std::cout << "iop " << (*iop) << std::endl;
    feed("iop", "factor", *iop);
    feed("iop", "factor", *iop);
    std::cout << std::endl;

    auto ia = dtd.a.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    std::cout << "ia " << (*ia) << std::endl;
    feed("ia", CHARDATA_NAME, *ia);
    feed("ia", "factor", *ia);
    std::cout << std::endl;
#else
    /*auto ifoo = dtd.foo.iterator();*/
    /*debug_log << __FILE__ << ':' << __LINE__ << debug_endl;*/
    /*std::cout << "ifoo " << (*ifoo) << std::endl;*/
    /*feed("ifoo", "a", *ifoo);*/
    /*feed("ifoo", "b", *ifoo);*/
    /*feed("ifoo", "op", *ifoo);*/
    /*feed("ifoo", "op", *ifoo);*/
    {
        std::cout << "===============================================================================" << std::endl;
        std::stringstream iss;
        iss << "<i>42</i>";
        int* test = test1.parse(iss);
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
        std::cout << test << std::endl;
        std::cout << *test << std::endl;
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
        delete test;
        std::cout << "===============================================================================" << std::endl;
    }
    {
        std::cout << "===============================================================================" << std::endl;
        std::stringstream iss;
        iss << "<i value=\"42\"/>";
        int* test = test2.parse(iss);
        std::cout << *test << std::endl;
        delete test;
        std::cout << "===============================================================================" << std::endl;
    }
#if 1
    {
        std::stringstream iss(XML1);
        Foo* test = dtd.parse(iss);
        std::cout << test->a << ' ' << test->b << std::endl;
        delete test;
    }
    {
        std::stringstream iss(XML2);
        Foo* test = dtd.parse(iss);
        std::cout << test->a << ' ' << test->b << std::endl;
        delete test;
    }
    {
        std::stringstream iss(XML3);
        Foo* test = dtd.parse(iss);
        std::cout << test->a << ' ' << test->b << std::endl;
        delete test;
    }
#endif
#endif
    return 0;
}

