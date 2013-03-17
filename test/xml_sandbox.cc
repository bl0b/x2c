#include <sstream>
#include <fstream>
#include <expat.h>


#include "XML_base.h"
#include "XML_iterators.h"
#include "XML_databinding.h"


#define BUFF_SIZE 4096

struct xml_context_impl {
    std::stringstream buffer;
    XML_Parser parser;
    xml_context_impl* parent;

    xml_context_impl(XML_Parser p, xml_context_impl* parent_context)
        : buffer()
        , parser(p)
        , parent(parent_context)
    {}

    virtual ~xml_context_impl()
    {
        XML_SetUserData(parser, static_cast<void*>(parent));
    }

    virtual bool consume_attribute(const std::string& name, const std::string& value) = 0;
    virtual bool consume_chardata(const std::string& value) = 0;
    virtual bool consume_element(const std::string& name) = 0;
    virtual void finish() = 0;
};

#if 1

template <typename EvalType>
    struct xml_context : public xml_context_impl {
        std::string text;
        EvalType* data;
        bool on_element;
        iterator_base<EvalType>* iter;
        std::function<void()> after;
        std::function<bool(const std::string&)> consume;

        xml_context(const xml_context<EvalType>& e) = delete;

        template <typename ParentEvalType, typename A, typename B, typename C> 
        xml_context(XML_Parser parser,
                    xml_context<ParentEvalType>* parent_context,
                    const data_binder<A, B, C>& binder)
            : xml_context_impl(parser, parent_context)
            , text()
            , on_element(false)
        {
            if (parent_context) {
                data = binder.install(parent_context->data);
                after = [binder, this, parent_context] () { binder.after(parent_context->data, data); };
                debug_log << "defined after()" << debug_endl;
            } else {
                data = binder.install(NULL);
                after = [] () {};
                debug_log << "didn't define after()" << debug_endl;
            }


            debug_log << "element address=" << binder.elt << debug_endl;
            debug_log << "element iterator() ? " << ((bool)binder.elt->iterator) << debug_endl;
            iter = binder.elt->iterator();
            debug_log << "blaaaaah" << debug_endl;

            consume = [this] (const std::string& name)
            {
                debug_log << "CONSUME " << name << std::endl;
                return iter->consume(name, this);
            };
        }

        void finish()
        {
            consume_chardata(buffer.str());
            /* FIXME : check iterator state and chardata consumption status */
            after();
        }

        ~xml_context()
        {
            delete iter;
        }

        bool consume_attribute(const std::string& name, const std::string& value)
        {
            on_element = false;
            text = value;
            return consume(name);
        }

        bool consume_chardata(const std::string& value)
        {
            static std::string chardata(CHARDATA_NAME);
            on_element = false;
            text = value;
            debug_log << "invoking chardata handler " << text << debug_endl;
            return consume(chardata);
        }

        bool consume_element(const std::string& name)
        {
            on_element = true;
            return consume(name);
        }

        template <typename SubOutputType, typename EntityType>
            void install(const data_binder<EvalType, SubOutputType, EntityType>& binder)
            {
                typedef typename std::remove_reference<decltype(*binder.install(data))>::type SubContextEvalType;
                DEBUG;
                if (on_element) {
                    XML_SetUserData(parser, static_cast<void*>(
                                new xml_context<SubContextEvalType>(
                                    parser,
                                    this,
                                    binder)));



                } else {
                    /* ERROR */
                }
            }

        template <typename SubOutputType>
            void install(const data_binder<EvalType, SubOutputType, std::string>& binder)
            {
                DEBUG;
                if (on_element) {
                    /* ERROR */
                }
                binder.after(binder.install(data), &text);
            }
    };

struct XMLReader {
    XML_Parser parser;

    XMLReader(const XML_Char* encoding="UTF-8")
        : parser(XML_ParserCreate(encoding))
    {
        XML_SetElementHandler(parser, start_hnd, end_hnd);
        XML_SetElementHandler(parser, start_hnd, end_hnd);
        XML_SetCharacterDataHandler(parser, chardata_hnd);
        XML_SetCdataSectionHandler(parser, cdata_start_hnd, cdata_end_hnd);
    }

    ~XMLReader()
    {
        XML_ParserFree(parser);
    }

    template <typename EvalType>
        struct _root { EvalType* ptr; };

    template <typename EvalType>
    EvalType* parse_from(const Element<EvalType>& root, std::istream& is)
    {
        DEBUG;
        Element<_root<EvalType>> pseudo_root("");
        pseudo_root = E(root, &_root<EvalType>::ptr);
        data_binder<void, _root<EvalType>, Element<_root<EvalType>>> root_binding("", &pseudo_root);
        XML_SetUserData(parser, static_cast<void*>(
            new xml_context<_root<EvalType>>(parser, static_cast<xml_context<void>*>(nullptr), root_binding)
        ));
        parse(is);
        return root_binding.data->ptr;
    }

    static bool eat_attributes(XML_Parser parser, const XML_Char** attributes)
    {
        xml_context_impl* ctx = static_cast<xml_context_impl*>(XML_GetUserData(parser));
        bool ok = true;
        while (*attributes && ok) {
            std::string key(*attributes++);
            std::string value(*attributes++);
            ok = ctx->consume_attribute(key, value);
            if (!ok) {
                /* ERROR */
            }
        }
        return ok;
    }

    void parse(std::istream& is)
    {
        while (!is.eof()) {
            void *buff = XML_GetBuffer(parser, BUFF_SIZE);
            is.read((char*) buff, BUFF_SIZE);
            XML_ParseBuffer(parser, is.gcount(), is.eof());
        }
    }

    void parse(const char* path)
    {
        std::ifstream f(path);
        parse(f);
    }

    static void start_hnd(void* userData, const XML_Char* name, const XML_Char** attrs) {
        debug_log << "start " << name << std::endl;
        xml_context_impl* context = static_cast<xml_context_impl*>(userData);
        if (!context->consume_element(name)) {
            /* ERROR */
        }
        if (!eat_attributes(context->parser, attrs)) {
            /* ERROR */
        }
        /*static_cast<XMLReader*>(userData)->_start(name, attrs);*/
    }
    static void end_hnd(void* userData, const XML_Char* name) {
        debug_log << "end " << name << std::endl;
        xml_context_impl* context = static_cast<xml_context_impl*>(userData);
        context->finish();
        delete context;
    }
    static void cdata_start_hnd(void* userData) {
        std::cout << "CDATA start" << std::endl;
        (void)userData;
    }
    static void cdata_end_hnd(void* userData) {
        std::cout << "CDATA end" << std::endl;
        (void)userData;
    }
    static void chardata_hnd(void* userData, const XML_Char* data, int len) {
        debug_log << "chardata " << std::string(data, data + len) << std::endl;
        static_cast<xml_context_impl*>(userData)->buffer << std::string(data, data + len);
    }
};


#endif






resolve_bindings<int> test;
struct check {};
resolve_bindings<check> test_compound;


template <typename EvalType>
struct Element : public Entity<EvalType> {
    using Entity<EvalType>::eval_type;
    using Entity<EvalType>::name;
    typedef transformer<resolve_bindings<EvalType>> linker;

    std::function<iterator_base<EvalType>*()> iterator;

    Element(const char* n)
        : Entity<EvalType>(n)
        , iterator()
    {}

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
            debug_log << "beware the " << typeid(transformed).name() << debug_endl;
            iterator = [=] () {
                debug_log << "beware the " << typeid(transformed).name() << debug_endl;
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

#define DTD_START(dtd_name, root_name, eval_type) \
    struct dtd_name ## _type { \
        Element<eval_type> root_name; \
        const Element<eval_type>& root() const { return root_name; } \
        eval_type* parse(std::istream& is) \
        { \
            DEBUG; \
            return XMLReader().parse_from(root_name, is); \
        } \
        dtd_name ## _type () \
            : root_name(#root_name) \
        {

#define ELEMENT(elt_name, eval_type) Element<eval_type> elt_name(#elt_name)

#define DTD_END(dtd_name) \
        } \
    } dtd_name

#if 1
DTD_START(dtd, foo, Foo)
    ELEMENT(a, int);
    ELEMENT(b, double);
    ELEMENT(factor, double);
    ELEMENT(op, Manip);
    /*ELEMENT(foo, Foo);*/

    a = chardata();
    b = A("value");
    factor = chardata();
    op = E(factor, &Manip::factor);
    foo = E(a, &Foo::a) & E(b, &Foo::b) & make_optional(E(op));
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
#endif
#endif
    return 0;
}

