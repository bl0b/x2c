/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef _X2C_H_
#define _X2C_H_

#include <fstream>
#include <sstream>
#include <expat.h>
#include <exception>

#include "XML_base.h"
#include "XML_iterators.h"
#include "XML_databinding.h"


#define BUFF_SIZE 4096

namespace x2c {

struct xml_exception : public std::exception {
    int line, col;
    std::string msg;

    xml_exception(XML_Parser parser, std::string custom_msg)
        : std::exception()
        , line(XML_GetCurrentLineNumber(parser))
        , col(XML_GetCurrentColumnNumber(parser))
    {
        std::stringstream s;
        s << "Error at line " << line << " column " << col;
        if (custom_msg.size()) {
            s << ": " << custom_msg;
        }
        msg = s.str();
    }

    const char* what() const throw() { return msg.c_str(); }
};


struct xml_context_impl {
    std::stringstream buffer;
    XML_Parser parser;
    xml_context_impl* parent;
    std::function<void()> rollback;

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

    void error(const std::string& msg) const
    {
        xml_exception e(parser, msg);
        X2C_ERR("ERROR: " << e.what() << std::endl);
        throw e;
    }

    void error(const char* msg="") const
    {
        xml_exception e(parser, msg);
        X2C_ERR("ERROR: " << e.what() << std::endl);
        throw e;
    }
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

        /* ajouter un paramètre pour l'itérateur courant, pour pouvoir invoquer dessus invalidate() si le prédicat final foire */

        template <typename ParentEvalType, typename A, typename B, typename C, typename kls> 
        xml_context(XML_Parser parser,
                    xml_context<ParentEvalType>* parent_context,
                    const data_binder<A, B, C>& binder,
                    iterator<ParentEvalType, kls, data_binder<A, B, C>>* binder_iter)
            : xml_context_impl(parser, parent_context)
            , text()
            , on_element(false)
        {
            if (parent_context) {
                data = binder.install(parent_context->data);
                after = [binder, this, parent_context, binder_iter] () {
                    DEBUG;
                    /* sûrement binder.after() devrait accepter le paramètre supplémentaire {iterator} pour appeler directement iterator.invalidate() */
                    if (!binder.after(parent_context->data, data)) {
                        binder_iter->invalidate();
                        binder.rollback(&data);
                    }
                };
                /*DEBUG_LOG("defined after()" << std::endl);*/
            } else {
                data = binder.install((ParentEvalType*)NULL);
                after = [] () {};
                /*DEBUG_LOG("didn't define after()" << std::endl);*/
            }

            rollback = [binder, this] ()
            {
                /*X2C_ERR(THIS << " ROLLBACK" << STD::ENDL);*/
                binder.rollback(&data);
                after = [] () {};
            };

            /*DEBUG_LOG("element address=" << binder.elt << std::endl);*/
            /*DEBUG_LOG("element iterator() ? " << ((bool)binder.elt->iterator) << std::endl);*/
            iter = binder.elt->iterator();

            consume = [this] (const std::string& name)
            {
                DEBUG_LOG("CONSUME " << name << std::endl);
                return iter->consume(name, this);
            };
        }

        void finish()
        {
            consume_chardata(buffer.str());
            /* FIXME : check iterator state and chardata consumption status */
            DEBUG_LOG("INVOKING AFTER()" << std::endl);
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
                DEBUG_LOG("invoking chardata handler " << text << std::endl);
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

        template <typename SubOutputType, typename EntityType, typename kls>
            void install(const data_binder<ignore, SubOutputType, EntityType>& binder,
                         iterator<EvalType, kls, data_binder<ignore, SubOutputType, EntityType>>* binder_iter)
            {
                typedef typename std::remove_reference<decltype(*binder.install(data))>::type SubContextEvalType;
                DEBUG;
                if (on_element) {
                    XML_SetUserData(parser, static_cast<void*>(
                                new xml_context<SubContextEvalType>(
                                    parser,
                                    this,
                                    binder,
                                    binder_iter)));
                } else {
                    error("(internal) Trying to install element binding but not on an element");
                }
            }

        template <typename SubOutputType, typename kls>
            void install(const data_binder<typename attr_func<SubOutputType>::func_type, SubOutputType, std::string>& binder,
                         iterator<EvalType, kls, data_binder<typename attr_func<SubOutputType>::func_type, SubOutputType, std::string>>* binder_iter)
            {
                DEBUG;
                if (on_element) {
                    error("(internal) Trying to install attribute binding but not on an attribute");
                }
                if (!binder.after(data, binder.install(data), &text)) {
                    binder_iter->invalidate();
                }
            }

        template <typename SubOutputType, typename kls>
            void install(const data_binder<ignore, SubOutputType, std::string>& binder,
                         iterator<EvalType, kls, data_binder<ignore, SubOutputType, std::string>>* binder_iter)
            {
                DEBUG;
                if (on_element) {
                    error("(internal) Trying to install attribute binding but not on an attribute");
                }
                if (!binder.after(data, binder.install(data), &text)) {
                    binder_iter->invalidate();
                }
            }

        template <typename SubOutputType, typename EntityType, typename kls>
            void install(const data_binder<EvalType, SubOutputType, EntityType>& binder,
                         iterator<EvalType, kls, data_binder<EvalType, SubOutputType, EntityType>>* binder_iter)
            {
                typedef typename std::remove_reference<decltype(*binder.install(data))>::type SubContextEvalType;
                DEBUG;
                if (on_element) {
                    XML_SetUserData(parser, static_cast<void*>(
                                new xml_context<SubContextEvalType>(
                                    parser,
                                    this,
                                    binder,
                                    binder_iter)));
                } else {
                    error("(internal) Trying to install element binding but not on an element");
                }
            }

        template <typename SubOutputType, typename kls>
            void install(const data_binder<EvalType, SubOutputType, std::string>& binder,
                         iterator<EvalType, kls, data_binder<EvalType, SubOutputType, std::string>>* binder_iter)
            {
                DEBUG;
                if (on_element) {
                    error("(internal) Trying to install attribute binding but not on an attribute");
                }
                if (!binder.after(data, binder.install(data), &text)) {
                    binder_iter->invalidate();
                }
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
        /*XML_SetCdataSectionHandler(parser, cdata_start_hnd, cdata_end_hnd);*/
        XML_SetUserData(parser, NULL);
    }

    ~XMLReader()
    {
        XML_ParserFree(parser);
    }

    template <typename EvalType>
        struct _root { EvalType* ptr; _root() : ptr(nullptr) {} };

    template <typename EvalType>
    EvalType* parse_from(const Element<EvalType>& root, std::istream& is)
    {
        DEBUG;
        Element<_root<EvalType>> pseudo_root("");
        pseudo_root = E(root, &_root<EvalType>::ptr);
        typedef data_binder<void, _root<EvalType>, Element<_root<EvalType>>> root_binder_type;
        root_binder_type root_binding("", &pseudo_root);
        typedef iterator<void, single, root_binder_type> root_iter_type;
        xml_context<_root<EvalType>>* context = new xml_context<_root<EvalType>>(parser,
                                                                                 static_cast<xml_context<void>*>(nullptr),
                                                                                 root_binding,
                                                                                 static_cast<root_iter_type*>(nullptr));
        XML_SetUserData(parser, static_cast<void*>(context));
        try {
            parse(is);
        } catch(xml_exception& e) {
            rollback();
            throw e;
        }
        EvalType* ret = context->data->ptr;
        delete context->data;
        delete context;
        return ret;
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
                std::stringstream s;
                s << "Couldn't consume attribute " << key << "=\"" << value << '"';
                ctx->error(s.str());
            }
        }
        return ok;
    }

    void rollback()
    {
        xml_context_impl* data;
        do {
            data = static_cast<xml_context_impl*>(XML_GetUserData(parser));
            if (data && data->rollback) {
                data->rollback();
            }
            delete data;
        } while (data != NULL);
    }

    void parse(std::istream& is)
    {
        while (is.good() && !is.eof()) {
            void *buff = XML_GetBuffer(parser, BUFF_SIZE);
            is.read((char*) buff, BUFF_SIZE);
            if (!XML_ParseBuffer(parser, is.gcount(), is.eof())) {
                static_cast<xml_context_impl*>(XML_GetUserData(parser))->error("Failed to parse buffer");
            }
        }
        if (!is.eof()) {
            static_cast<xml_context_impl*>(XML_GetUserData(parser))->error("Failed to parse buffer");
        }
    }

    void parse(const char* path)
    {
        std::ifstream f(path);
        parse(f);
    }

    static void start_hnd(void* userData, const XML_Char* name, const XML_Char** attrs) {
        DEBUG_LOG("start " << name << std::endl);
        xml_context_impl* context = static_cast<xml_context_impl*>(userData);
        if (!context->consume_element(name)) {
            context->error("Failed to consume element");
        }
        if (!eat_attributes(context->parser, attrs)) {
            context->error("Failed to consume attributes");
        }
        /*static_cast<XMLReader*>(userData)->_start(name, attrs);*/
    }
    static void end_hnd(void* userData, const XML_Char* name) {
        DEBUG_LOG("end " << name << std::endl);
        xml_context_impl* context = static_cast<xml_context_impl*>(userData);
        context->finish();
        delete context;
    }
    /*static void cdata_start_hnd(void* userData) {*/
        /*X2C_OUT("CDATA START" << STD::ENDL);*/
        /*(void)userData;*/
    /*}*/
    /*static void cdata_end_hnd(void* userData) {*/
        /*X2C_OUT("CDATA END" << STD::ENDL);*/
        /*(void)userData;*/
    /*}*/
    static void chardata_hnd(void* userData, const XML_Char* data, int len) {
        DEBUG_LOG("chardata " << std::string(data, data + len) << std::endl);
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

}


#define DTD_FORWARD_DECL(dtd_name, eval_type) struct dtd_name ## _type; const ::x2c::Element<eval_type>& dtd_name ## _root();

#define DTD_START_WITH_ROOT_NAME(dtd_name, root_name, root_xml_name, eval_type) \
    struct dtd_name ## _type { \
        typedef eval_type type; \
        ::x2c::Element<type> root_name; \
        const ::x2c::Element<eval_type>& root() const { return root_name; } \
        eval_type* parse(std::istream& is) \
        { \
            DEBUG; \
            return ::x2c::XMLReader().parse_from(root_name, is); \
        } \
        dtd_name ## _type () \
            : root_name(root_xml_name) \
        {

#define DTD_START(dtd_name, root_name, eval_type) DTD_START_WITH_ROOT_NAME(dtd_name, root_name, #root_name, eval_type)

#define ELEMENT_WITH_NAME(elt_name, elt_xml_name, eval_type) static ::x2c::Element<eval_type> elt_name(elt_xml_name)
#define ELEMENT(elt_name, eval_type) ELEMENT_WITH_NAME(elt_name, #elt_name, eval_type)


#define DTD_END(dtd_name) \
        } \
    } dtd_name;

#define DTD_ACCESSOR(dtd_name) const ::x2c::Element<dtd_name ## _type :: type>& dtd_name ## _root() { return dtd_name.root(); }



#endif
