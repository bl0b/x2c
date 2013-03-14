#include "XML_base.h"

#include <expat.h>

template <typename EvalType>
struct xml_parser {
    XML_Parser parser;
    EvalType result;

    xml_parser(/*Element_Base* root_element, */const XML_Char* encoding="UTF-8")
        : parser(XML_ParserCreate(encoding))
        , result()
    {
        XML_SetElementHandler(parser, start_hnd, end_hnd);
        XML_SetCharacterDataHandler(parser, chardata_hnd);
        XML_SetCdataSectionHandler(parser, cdata_start_hnd, cdata_end_hnd);
        XML_SetUserData(parser, static_cast<void*>(this));
    }

    template <typename OtherEvalType>
        xml_parser(xml_parser<OtherEvalType>& parent)
        : parser(parent)
        , result()


    static void start_hnd(void* userData, const XML_Char* name, const XML_Char** attrs) {
        
    }
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
                return make_iterator(transformed);
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


#if 1
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
    }
};
#endif


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
    dtd d;
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    auto iop = d.op.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    auto ifoo = d.foo.iterator();
    debug_log << __FILE__ << ':' << __LINE__ << debug_endl;
    std::cout << "iop " << (*iop) << std::endl;
    feed("iop", "factor", *iop);
    feed("iop", "factor", *iop);
    std::cout << std::endl;
    std::cout << "ifoo " << (*ifoo) << std::endl;
    feed("ifoo", "a", *ifoo);
    feed("ifoo", "b", *ifoo);
    feed("ifoo", "op", *ifoo);
    feed("ifoo", "op", *ifoo);
#endif
    return 0;
}

