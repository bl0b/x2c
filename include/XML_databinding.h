/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef _X2C_DATABINDING_H_
#define _X2C_DATABINDING_H_

namespace x2c {

template <typename T>
    struct delete_on_scope_exit {
        T* data;
        ~delete_on_scope_exit() { delete data; }
    };

template <typename RootType>
struct data_binder<void, RootType, Element<RootType>> {
    typedef Element<RootType> entity_type;

    const std::string name;
    const entity_type* elt;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e)
    {}

    RootType* install(void* container) const
    {
        return new RootType();
        (void)container;
    }

    bool after(void* ptr, RootType* data) const
    {
        DEBUG;
        return true;
        (void)ptr; (void)data;
    }

    void rollback(RootType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        delete *ptr;
        *ptr = nullptr;
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
    }
};


/* no binding : validate and discard element */
template <typename EvalType>
struct data_binder<ignore, EvalType, Element<EvalType>> {
    typedef Element<EvalType> entity_type;
    typedef EvalType value_type;
    const std::string name;
    const entity_type* elt;

    data_validator<value_type> validate;

    data_binder(const std::string& n, const entity_type* e, data_validator<value_type> v)
        : name(n), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    template <typename T>
    EvalType* install(T* container) const
    {
        return new EvalType();
    }

    template <typename T>
    bool after(T* container, EvalType* data) const
    {
        delete_on_scope_exit<value_type> _ = { data };
        return validate(data);
    }

    void rollback(EvalType** ptr) const
    {
        delete *ptr;
        *ptr = NULL;
    }
};


/* simple setter */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;
    typedef FieldType value_type;

    const std::string name;
    FieldType StrucType::* target;
    const entity_type* elt;

    data_validator<value_type> validate;

    data_binder(const std::string& n, FieldType StrucType::* t, const entity_type* e, data_validator<value_type> v)
        : name(n), target(t), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, Element<StrucType>>& d)
        : name(d.name), target(d.target), elt(d.elt), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    FieldType* install(StrucType* container) const
    {
        /*return &(container->*target);*/
        return new FieldType();
    }

    bool after(StrucType* container, FieldType* data) const
    {
        DEBUG;
        delete_on_scope_exit<FieldType> _ = { data };
        if (validate(data)) {
            container->*target = *data;
            return true;
        }
        return false;
    }

    void rollback(FieldType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        (void)ptr;
    }
};


/* simple setter with allocator */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType* StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;
    typedef FieldType value_type;

    const std::string name;
    FieldType* StrucType::* target;
    const entity_type* elt;

    data_validator<value_type> validate;

    data_binder(const std::string& n, FieldType* StrucType::* t, const entity_type* e, data_validator<value_type> v)
        : name(n), target(t), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<StrucType, FieldType* StrucType::*, Element<StrucType>>& d)
        : name(d.name), target(d.target), elt(d.elt), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    FieldType* install(StrucType* container) const
    {
        DEBUG;
        debug_log << "container=" << container << " target=" << container->*target << debug_endl;
        return new FieldType();
    }

    bool after(StrucType* container, FieldType* data) const
    {
        DEBUG;
        debug_log << "container=" << container << " \"" << elt->name << "\" target=" << container->*target << debug_endl;
        if (validate(data)) {
            container->*target = data;
            return true;
        } else {
            delete data;
        }
        return false;
    }

    void rollback(FieldType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        delete *ptr;
        *ptr = nullptr;
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
    }
};


/* manipulator (target type is not related to element::eval_type) */
template <typename EvalType, typename Manipulator>
struct data_binder<EvalType, Manipulator, Element<Manipulator>> {
    typedef Element<Manipulator> entity_type;

    const std::string name;
    const entity_type* elt;

    typedef Manipulator value_type;

    data_validator<value_type> validate;

    data_binder(const std::string& n, const entity_type* e, data_validator<value_type> v)
        : name(n), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<EvalType, Manipulator, Element<Manipulator>>& d)
        : name(d.name), elt(d.elt), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    Manipulator* install(EvalType* container) const
    {
        return new Manipulator();
        (void)container;
    }

    bool after(EvalType* container, Manipulator* data) const
    {
        DEBUG;
        delete_on_scope_exit<Manipulator> _ = { data };
        debug_log << "container=" << container << " \"" << elt->name << "\" manip=" << data << debug_endl;
        if (validate(data)) {
            (*data)(*container);
            return true;
        } else {
            return false;
        }
    }

    void rollback(Manipulator** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        delete *ptr;
        *ptr = nullptr;
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
    }
};


/* transient binding (target is forwarded) */
template <typename EvalType>
struct data_binder<EvalType, EvalType, Element<EvalType>> {
    typedef Element<EvalType> entity_type;

    const std::string name;
    const entity_type* elt;

    typedef EvalType value_type;

    data_validator<value_type> validate;

    data_binder(const std::string& n, const entity_type* e, data_validator<value_type> v)
        : name(n), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<EvalType, EvalType, Element<EvalType>>& d)
        : name(d.name), elt(d.elt), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    EvalType* install(EvalType* container) const
    {
        return container;
    }

    bool after(EvalType* container, EvalType* data) const
    {
        DEBUG;
        return validate(data);
    }

    void rollback(EvalType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        (void)ptr;
    }
};


/* attribute and chardata eval_to binding */
template <typename EvalType>
struct data_binder<EvalType, EvalType, std::string> {
    const std::string name;
    typedef std::string entity_type;
    typedef EvalType value_type;

    data_validator<value_type> validate;

    data_binder(const std::string& n)
        : name(n), validate([] (value_type*) { return true; })
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<EvalType, EvalType, std::string>& d)
        : name(d.name), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    EvalType* install(EvalType* container) const
    {
        return new EvalType();
    }

    bool after(EvalType* container, EvalType* ptr, std::string* data) const
    {
        DEBUG;
        delete_on_scope_exit<EvalType> _ = { ptr };
        from_string(*data, *ptr);
        if (validate(ptr)) {
            *container = *ptr;
            return true;
        }
        return false;
        /*debug_log << "converted => " << (*container) << debug_endl;*/
        /*return validate(container);*/
    }

    void rollback(EvalType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        (void)ptr;
    }
};

/* attribute and chardata transformation binding */
template <typename EvalType>
struct data_binder<typename attr_func<EvalType>::func_type, EvalType, std::string> {
    const std::string name;
    typedef std::string entity_type;
    typedef EvalType value_type;
    typedef typename attr_func<EvalType>::func_type func_type;

    data_validator<value_type> validate;
    func_type transformer;

    data_binder(const std::string& n, func_type f)
        : name(n), validate([] (value_type*) { return true; }), transformer(f)
    {
        if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; }
        if (!transformer) { std::cerr << "INVALID TRANSFORMER " << __FILE__ << ':' << __LINE__ << std::endl; }
    }

    data_binder(const data_binder<typename attr_func<EvalType>::func_type, EvalType, std::string>& d)
        : name(d.name), validate(d.validate), transformer(d.transformer)
    {
        if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; }
        if (!transformer) { std::cerr << "INVALID TRANSFORMER " << __FILE__ << ':' << __LINE__ << std::endl; }
    }

    EvalType* install(EvalType* container) const
    {
        /*return new EvalType();*/
        return container;
    }

    bool after(EvalType* container, EvalType* ptr, std::string* data) const
    {
        DEBUG;
        /*delete_on_scope_exit<EvalType> _ = { ptr };*/
        /*from_string(*data, *ptr);*/
        if (transformer(data, container) && validate(container)) {
            return true;
        }
        return false;
        /*debug_log << "converted => " << (*container) << debug_endl;*/
        /*return validate(container);*/
    }

    void rollback(EvalType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        (void)ptr;
    }
};


/* no binding : validate and discard attribute/chardata */
template <typename EvalType>
struct data_binder<ignore, EvalType, std::string> {
    typedef Element<EvalType> entity_type;
    typedef std::string value_type;

    const std::string name;
    const entity_type* elt;

    data_validator<value_type> validate;

    data_binder(const std::string& n, data_validator<value_type> v)
        : name(n), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    template <typename T>
    EvalType* install(T* container) const
    {
        return NULL;
    }

    template <typename T>
    bool after(T* container, EvalType* ptr, std::string* data) const
    {
        return validate(data);
        (void)container; (void)ptr;
    }

    void rollback(EvalType** ptr) const
    {
        (void)ptr;
    }
};


/* attribute binding */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, std::string> {
    const std::string name;
    FieldType StrucType::* field;
    typedef std::string entity_type;
    typedef FieldType value_type;

    data_validator<value_type> validate;

    data_binder(const std::string& n, FieldType StrucType::* f, data_validator<value_type> v)
        : name(n), field(f), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, std::string>& d)
        : name(d.name), field(d.field), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    FieldType* install(StrucType* container) const
    {
        /*return &(container->*field);*/
        return new FieldType();
    }

    bool after(StrucType* container, FieldType* ptr, std::string* data) const
    {
        DEBUG;
        delete_on_scope_exit<FieldType> _ = { ptr };
        from_string(*data, *ptr);
        if (validate(ptr)) {
            container->*field = *ptr;
            return true;
        }
        return false;
        /*debug_log << "converted => " << (*ptr) << debug_endl;*/
        /*return validate(ptr);;*/
    }

    void rollback(FieldType** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        *ptr = NULL;
    }
};


/* container::value_type binding (special case for value_types like std::pair<const K, V>) */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, Element<typename unconst_value_type<typename FieldType::value_type>::type>> {
    typedef typename unconst_value_type<typename FieldType::value_type>::type value_type;
    typedef Element<value_type> entity_type;

    const std::string name;
    FieldType StrucType::* coll;
    const entity_type* elt;

    data_validator<value_type> validate;

    data_binder(const std::string& n, FieldType StrucType::* f, const entity_type* e, data_validator<value_type> v)
        : name(n), coll(f), elt(e), validate(v)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, std::string>& d)
        : name(d.name), coll(d.field), validate(d.validate)
    { if (!validate) { std::cerr << "INVALID VALIDATOR " << __FILE__ << ':' << __LINE__ << std::endl; } }

    value_type* install(StrucType* container) const
    {
        return new value_type();
    }

    bool after(StrucType* ptr, value_type* data) const
    {
        DEBUG;
        delete_on_scope_exit<value_type> _ = { data };
        if (validate(data)) {
            select_insertion_method<FieldType>::add(ptr->*coll, *data);
            return true;
        } else {
            return false;
        }
    }

    void rollback(value_type** ptr) const
    {
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
        delete *ptr;
        *ptr = nullptr;
        /*std::cerr << "ROLLBACK " << __FILE__ << ':' << __LINE__ << ' ' << ptr << ' ' << (*ptr) << std::endl;*/
    }
};


#if 1
template <typename kls, typename BindDescr, typename Predicate>
combination<kls, BindDescr>
operator / (combination<kls, BindDescr> comb,
            Predicate val)
{
    std::get<0>(comb) /= val;
    return comb;
    /*return { std::get<0>(comb) / val };*/
}
#endif


template <typename X> struct _true_ {
    static bool _(X*) { return true; }
};

inline
combination<single, CharData>
chardata() { return { CharData() }; }


inline
combination<single, attr_eval>
A(const char* name)
{
    return { { name } };
}

template <typename EvalType>
combination<single, attr_func<EvalType>>
A(const char* name, std::function<bool(const std::string*, EvalType*)> f)
{
    return { { name, f } };
}


template <typename StrucType, typename FieldType>
combination<single, attr_binding<StrucType, FieldType>>
A(const char* name, FieldType StrucType::* field)
{
    data_validator<FieldType> k = &_true_<FieldType>::_;
    return { attr_binding<StrucType, FieldType>(name, field, k) };
}


inline
combination<single, attr_binding<ignore, std::string>>
A(const char* name, ignore_entity ignore::* field)
{
    data_validator<std::string> k = &_true_<std::string>::_;
    return { attr_binding<ignore, std::string>(name, k) };
}


template <typename StrucType, typename FieldType>
combination<single, elt_binding<StrucType, FieldType>>
E(const Element<FieldType>& elt, FieldType StrucType::* field)
{
    data_validator<FieldType> k = &_true_<FieldType>::_;
    return {
        elt_binding<StrucType, FieldType>(&elt, field, k)
    };
}


template <typename StrucType, typename FieldType>
combination<single, elt_alloc_binding<StrucType, FieldType>>
E(const Element<FieldType>& elt, FieldType* StrucType::* field)
{
    data_validator<FieldType> k = &_true_<FieldType>::_;
    return {
        { &elt, field, k }
    };
}


template <typename FieldType>
combination<single, elt_binding<ignore, FieldType>>
E(const Element<FieldType>& elt, ignore_entity ignore::* field)
{
    data_validator<FieldType> k = &_true_<FieldType>::_;
    return {
        { &elt, k }
    };
}





template <typename StrucType, class CollType>
typename std::enable_if<
    is_container<CollType>::value,
    combination<multiple, elt_coll_binding<StrucType, CollType>>
>::type
E(const Element<typename unconst_value_type<typename CollType::value_type>::type>& elt, CollType StrucType::* field)
{
    data_validator<typename unconst_value_type<typename CollType::value_type>::type> k = &_true_<typename unconst_value_type<typename CollType::value_type>::type>::_;
    return {
        { &elt, field, k }
    };
}


template <typename ManipulatorOrTransient>
combination<single, const Element<ManipulatorOrTransient>*>
E(const Element<ManipulatorOrTransient>& elt)
{
    return { &elt };
}

template <typename Anything>
combination<multiple, Anything>
M(const combination<single, Anything>& c)
{
    return { std::get<0>(c) };
}


template <typename IntegralType>
struct resolve_bindings_integral {
    /* manipulator */
    /* transient */
    template <typename ManipulatorOrTransient>
        static
        data_binder<IntegralType, ManipulatorOrTransient, Element<ManipulatorOrTransient>>
        transform(const Element<ManipulatorOrTransient>* e)
        {
            DEBUG;
            return { e->name, e, &_true_<ManipulatorOrTransient>::_ };
        }

    /* attr func */
    static
    data_binder<typename attr_func<IntegralType>::func_type, IntegralType, std::string>
    transform(const attr_func<IntegralType> & ae)
    {
        DEBUG;
        return { ae.name, ae.func };
    }

    /* attr eval_to */
    static
    data_binder<IntegralType, IntegralType, std::string>
    transform(const attr_eval & ae)
    {
        DEBUG;
        return { ae.name };
    }

    /* chardata eval_to */
    static
    data_binder<IntegralType, IntegralType, std::string>
    transform(const CharData &)
    {
        DEBUG;
        return { CHARDATA_NAME };
    }

    /* ignore */
    template <typename X>
        static
        data_binder<ignore, IntegralType, Element<X>>
        transform(const elt_binding<ignore, X>& eb)
        {
            DEBUG;
            return data_binder<ignore, IntegralType, Element<X>>(eb.elt->name, eb.validate);
        }

    /* ignore */
    static
    data_binder<ignore, IntegralType, std::string>
    transform(const attr_binding<ignore, std::string>& ab)
    {
        DEBUG;
        return { ab.name, ab.validate };
    }
};

template <typename StrucType>
struct resolve_bindings_class {
    /* manipulator */
    /* transient */
    template <typename ManipulatorOrTransient>
        static
        data_binder<StrucType, ManipulatorOrTransient, Element<ManipulatorOrTransient>>
        transform(const Element<ManipulatorOrTransient>* e)
        {
            DEBUG;
            debug_log << e->name << debug_endl;
            return { e->name, e, &_true_<ManipulatorOrTransient>::_ };
        }

    /* attr func */
    static
    data_binder<typename attr_func<StrucType>::func_type, StrucType, std::string>
    transform(const attr_func<StrucType> & ae)
    {
        DEBUG;
        return { ae.name, ae.func };
    }

    /* attr eval_to */
    static
    data_binder<StrucType, StrucType, std::string>
    transform(const attr_eval & ae)
    {
        DEBUG;
        return { ae.name };
    }

    /* chardata eval_to */
    static
    data_binder<StrucType, StrucType, std::string>
    transform(const CharData &)
    {
        DEBUG;
        return { CHARDATA_NAME };
    }


    /* simple setter */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType StrucType::*, Element<FieldType>>
        transform(const elt_binding<StrucType, FieldType>& eb)
        {
            DEBUG;
            return { eb.elt->name, eb.field, eb.elt, eb.validate };
        }

    /* simple setter with allocator */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType* StrucType::*, Element<FieldType>>
        transform(const elt_alloc_binding<StrucType, FieldType>& eb)
        {
            DEBUG;
            return { eb.elt->name, eb.field, eb.elt, eb.validate };
        }

    /* attr binding */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType StrucType::*, std::string>
        transform(const attr_binding<StrucType, FieldType>& ab)
        {
            DEBUG;
            return data_binder<StrucType, FieldType StrucType::*, std::string>(ab.name, ab.field, ab.validate);
        }

    /* simple setter */
        template <class CollType>
        static
        data_binder<StrucType, CollType StrucType::*, Element<typename unconst_value_type<typename CollType::value_type>::type>>
        transform(const elt_coll_binding<StrucType, CollType>& eb)
        {
            DEBUG;
            return { eb.elt->name, eb.field, eb.elt, eb.validate };
        }

    /* ignore */
    template <typename X>
        static
        data_binder<ignore, X, Element<X>>
        transform(const elt_binding<ignore, X>& eb)
        {
            DEBUG;
            return data_binder<ignore, X, Element<X>>(eb.elt->name, eb.elt, eb.validate);
            /*return { eb.elt->name, eb.validate };*/
        }

    /* ignore */
    static
    data_binder<ignore, StrucType, std::string>
    transform(const attr_binding<ignore, std::string>& ab)
    {
        DEBUG;
        return { ab.name, ab.validate };
    }
};

template <typename SomeType>
struct resolve_bindings : public std::conditional<
                                     std::is_class<SomeType>::value,
                                     resolve_bindings_class<SomeType>,
                                     resolve_bindings_integral<SomeType>>::type
{};

}

#endif

