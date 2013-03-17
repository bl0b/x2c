#ifndef _XML_DATABINDING_H_
#define _XML_DATABINDING_H_

template <typename RootType>
struct data_binder<void, RootType, Element<RootType>> {
    typedef Element<RootType> entity_type;

    const std::string name;
    const entity_type* elt;
    RootType* data;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e), data(new RootType())
    {
        debug_log << "Root data=" << data << debug_endl;
    }

    RootType* install(void* container) const
    {
        return const_cast<RootType*>(data);
        (void)container;
    }

    void after(void* ptr, RootType* data) const
    {
        DEBUG;
        (void)ptr; (void)data;
    }
};


/* simple setter */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;

    const std::string name;
    FieldType StrucType::* target;
    const entity_type* elt;

    data_binder(const std::string& n, FieldType StrucType::* t, const entity_type* e)
        : name(n), target(t), elt(e)
    { DEBUG; }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, Element<StrucType>>& d)
        : name(d.name), target(d.target), elt(d.elt)
    { DEBUG; }

    FieldType* install(StrucType* container) const
    {
        return &(container->*target);
    }

    void after(StrucType* ptr, FieldType* data) const
    {
        DEBUG;
        debug_log << "container=" << ptr << " \"" << elt->name << "\" target=" << ptr->*target << debug_endl;
        (void)ptr; (void)data;
    }
};


/* simple setter with allocator */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType* StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;

    const std::string name;
    FieldType* StrucType::* target;
    const entity_type* elt;

    data_binder(const std::string& n, FieldType* StrucType::* t, const entity_type* e)
        : name(n), target(t), elt(e)
    { DEBUG; }

    data_binder(const data_binder<StrucType, FieldType* StrucType::*, Element<StrucType>>& d)
        : name(d.name), target(d.target), elt(d.elt)
    { DEBUG; }

    FieldType* install(StrucType* container) const
    {
        DEBUG;
        container->*target = new FieldType();
        debug_log << "container=" << container << " target=" << container->*target << debug_endl;
        return container->*target;
    }

    void after(StrucType* container, FieldType* data) const
    {
        DEBUG;
        debug_log << "container=" << container << " \"" << elt->name << "\" target=" << container->*target << debug_endl;
        (void)container; (void)data;
    }
};


/* manipulator (target type is not related to element::eval_type) */
template <typename EvalType, typename Manipulator>
struct data_binder<EvalType, Manipulator, Element<Manipulator>> {
    typedef Element<Manipulator> entity_type;

    const std::string name;
    const entity_type* elt;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e)
    {
        DEBUG;
        debug_log << ((void*)this) << ' ' << name << debug_endl;
    }

    data_binder(const data_binder<EvalType, Manipulator, Element<Manipulator>>& d)
        : name(d.name), elt(d.elt)
    { DEBUG; }

    Manipulator* install(EvalType* container) const
    {
        return new Manipulator();
        (void)container;
    }

    void after(EvalType* container, Manipulator* data) const
    {
        DEBUG;
        debug_log << "container=" << container << " \"" << elt->name << "\" manip=" << data << debug_endl;
        (*data)(*container);
        delete data;
    }
};


/* transient binding (target is forwarded) */
template <typename EvalType>
struct data_binder<EvalType, EvalType, Element<EvalType>> {
    typedef Element<EvalType> entity_type;

    const std::string name;
    const entity_type* elt;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e)
    { DEBUG; }

    data_binder(const data_binder<EvalType, EvalType, Element<EvalType>>& d)
        : name(d.name), elt(d.elt)
    { DEBUG; }

    EvalType* install(EvalType* container) const
    {
        return container;
    }

    void after(EvalType* container, EvalType* data) const
    { DEBUG; }
};


/* attribute and chardata eval_to binding */
template <typename EvalType>
struct data_binder<EvalType, EvalType, std::string> {
    const std::string name;
    typedef std::string entity_type;

    data_binder(const std::string& n)
        : name(n)
    { DEBUG; }

    data_binder(const data_binder<EvalType, EvalType, std::string>& d)
        : name(d.name)
    { DEBUG; }

    EvalType* install(EvalType* container) const
    {
        return container;
    }

    void after(EvalType* container, std::string* data) const
    {
        DEBUG;
        from_string(*data, *container);
        debug_log << "converted => " << (*container) << debug_endl;
    }
};


/* attribute binding */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, std::string> {
    const std::string name;
    FieldType StrucType::* field;
    typedef std::string entity_type;

    data_binder(const std::string& n, FieldType StrucType::* f)
        : name(n), field(f)
    { DEBUG; }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, std::string>& d)
        : name(d.name), field(d.field)
    { DEBUG; }

    FieldType* install(StrucType* container) const
    {
        return container->*field;
    }

    void after(FieldType* ptr, std::string* data) const
    {
        DEBUG;
        from_string(*data, *ptr);
        debug_log << "converted => " << (*ptr) << debug_endl;
    }
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


template <typename StrucType, typename FieldType>
combination<single, attr_binding<StrucType, FieldType>>
A(const char* name, FieldType StrucType::* field)
{
    return { { name, field } };
}


template <typename StrucType, typename FieldType>
combination<single, elt_binding<StrucType, FieldType>>
E(const Element<FieldType>& elt, FieldType StrucType::* field)
{
    return {
        { &elt, field }
    };
}


template <typename StrucType, typename FieldType>
combination<single, elt_alloc_binding<StrucType, FieldType>>
E(const Element<FieldType>& elt, FieldType* StrucType::* field)
{
    return {
        { &elt, field }
    };
}


template <typename ManipulatorOrTransient>
combination<single, const Element<ManipulatorOrTransient>*>
E(const Element<ManipulatorOrTransient>& elt)
{
    return { &elt };
}

template <typename StrucType, typename FieldType>
combination<optional<single>, attr_binding<StrucType, FieldType>>
OA(const char* name, FieldType StrucType::* field)
{
    return { { name, field } };
}


template <typename StrucType, typename FieldType>
combination<optional<single>, elt_binding<StrucType, FieldType>>
OE(const Element<FieldType>& elt, FieldType StrucType::* field)
{
    return {
        { &elt, field }
    };
}


template <typename ManipulatorOrTransient>
combination<optional<single>, const Element<ManipulatorOrTransient>*>
OE(const Element<ManipulatorOrTransient>& elt)
{
    return { &elt };
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
            return { e->name, e };
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
            return { e->name, e };
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
            return { eb.elt->name, eb.field, eb.elt };
        }

    /* simple setter with allocator */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType* StrucType::*, Element<FieldType>>
        transform(const elt_alloc_binding<StrucType, FieldType>& eb)
        {
            DEBUG;
            return { eb.elt->name, eb.field, eb.elt };
        }

    /* attr binding */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType StrucType::*, std::string>
        transform(const attr_binding<StrucType, FieldType>& ab)
        {
            DEBUG;
            return { ab.field };
        }
};

template <typename SomeType>
struct resolve_bindings : public std::conditional<
                                     std::is_class<SomeType>::value,
                                     resolve_bindings_class<SomeType>,
                                     resolve_bindings_integral<SomeType>>::type
{};

#endif
