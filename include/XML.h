#ifndef _XML_BIND_H_
#define _XML_BIND_H_

#include <sstream>
#include <expat.h>
#include <exception>

#include "XML_base.h"
#include "XML_iterators.h"
#include "XML_databinding.h"


#define BUFF_SIZE 4096

struct xml_exception : public std::exception {
    int line, col;
    std::string msg;

    xml_exception(XML_Parser parser)
        : std::exception()
        , line(XML_GetCurrentLineNumber(parser))
        , col(XML_GetCurrentColumnNumber(parser))
    {
        std::stringstream s;
        s << "Error at line " << line << " column " << col;
        msg = s.str();
    }

    const char* what() const throw() { return msg.c_str(); }
};

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
                after = [binder, this, parent_context] () {
                    DEBUG;
                    binder.after(parent_context->data, data);
                };
                /*debug_log << "defined after()" << debug_endl;*/
            } else {
                data = binder.install(NULL);
                after = [] () {};
                /*debug_log << "didn't define after()" << debug_endl;*/
            }


            /*debug_log << "element address=" << binder.elt << debug_endl;*/
            /*debug_log << "element iterator() ? " << ((bool)binder.elt->iterator) << debug_endl;*/
            iter = binder.elt->iterator();

            consume = [this] (const std::string& name)
            {
                debug_log << "CONSUME " << name << debug_endl;
                return iter->consume(name, this);
            };
        }

        void finish()
        {
            consume_chardata(buffer.str());
            /* FIXME : check iterator state and chardata consumption status */
            debug_log << "INVOKING AFTER()" << debug_endl;
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
            if (iter->accept(chardata)) {
                on_element = false;
                text = value;
                debug_log << "invoking chardata handler " << text << debug_endl;
                return consume(chardata);
            } else {
                return false;
            }
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
            if (!XML_ParseBuffer(parser, is.gcount(), is.eof())) {
                throw xml_exception(parser);
            }
        }
    }

    void parse(const char* path)
    {
        std::ifstream f(path);
        parse(f);
    }

    static void start_hnd(void* userData, const XML_Char* name, const XML_Char** attrs) {
        debug_log << "start " << name << debug_endl;
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
        debug_log << "end " << name << debug_endl;
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
        debug_log << "chardata " << std::string(data, data + len) << debug_endl;
        static_cast<xml_context_impl*>(userData)->buffer << std::string(data, data + len);
    }
};


template <typename EvalType>
struct Element : public Entity<EvalType> {
    using Entity<EvalType>::eval_type;
    using Entity<EvalType>::name;
    typedef transformer<resolve_bindings<EvalType>> linker;

    std::function<iterator_base<EvalType>*()> iterator;

    Element(const char* n)
        : Entity<EvalType>(n)
        , iterator()
    {
        iterator = [] () { return new iterator_empty<EvalType>(); };
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


#define ELEMENT(elt_name, eval_type) static Element<eval_type> elt_name(#elt_name)


#define DTD_END(dtd_name) \
        } \
    } dtd_name



#endif
