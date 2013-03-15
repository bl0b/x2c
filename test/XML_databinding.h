#ifndef _XML_DATABINDING_H_
#define _XML_DATABINDING_H_

/* simple setter */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;

    std::string name;
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
        return &container->*target;
    }

    void after(StrucType* ptr, FieldType data) const
    { DEBUG; }
};


/* simple setter with allocator */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType* StrucType::*, Element<FieldType>> {
    typedef Element<FieldType> entity_type;

    std::string name;
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
        container->*target = new FieldType();
        return container->*target;
    }

    void after(StrucType* container, FieldType data) const
    { DEBUG; }
};


/* manipulator (target type is not related to element::eval_type) */
template <typename EvalType, typename Manipulator>
struct data_binder<EvalType, Manipulator, Element<Manipulator>> {
    typedef Element<Manipulator> entity_type;

    std::string name;
    const entity_type* elt;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e)
    { DEBUG; }

    data_binder(const data_binder<EvalType, Manipulator, Element<Manipulator>>& d)
        : name(d.name), elt(d.elt)
    { DEBUG; }

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
template <typename EvalType>
struct data_binder<EvalType, EvalType, Element<EvalType>> {
    typedef Element<EvalType> entity_type;

    std::string name;
    const entity_type* elt;

    data_binder(const std::string& n, const entity_type* e)
        : name(n), elt(e)
    { DEBUG; }

    data_binder(const data_binder<EvalType, EvalType, Element<EvalType>>& d)
        : name(d.name), elt(d.elt)
    { DEBUG; }

    EvalType* install(EvalType* container)
    {
        return container;
    }

    void after(EvalType* container, EvalType* data)
    { DEBUG; }
};


/* attribute and chardata eval_to binding */
template <typename EvalType>
struct data_binder<EvalType, EvalType, std::string> {
    std::string name;
    typedef std::string entity_type;

    data_binder(const std::string& n)
        : name(n)
    { DEBUG; }

    data_binder(const data_binder<EvalType, EvalType, std::string>& d)
        : name(d.name)
    { DEBUG; }

    EvalType* install(EvalType* container)
    {
        return container;
    }

    void after(EvalType* container, EvalType* data)
    {
        from_string(*data, *container);
    }
};


/* attribute binding */
template <typename StrucType, typename FieldType>
struct data_binder<StrucType, FieldType StrucType::*, std::string> {
    std::string name;
    FieldType StrucType::* field;
    typedef std::string entity_type;

    data_binder(const std::string& n, FieldType StrucType::* f)
        : name(n), field(f)
    { DEBUG; }

    data_binder(const data_binder<StrucType, FieldType StrucType::*, std::string>& d)
        : name(d.name), field(d.field)
    { DEBUG; }

    FieldType* install(StrucType* container)
    {
        return container->*field;
    }

    void after(FieldType* ptr, std::string* data)
    {
        from_string(*data, *ptr);
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


#define CHARDATA_NAME "#CHARDATA"

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
    /* simple setter with allocator */
    template <typename FieldType>
        static
        data_binder<StrucType, FieldType StrucType::*, Element<FieldType>>
        transform(const elt_binding<StrucType, FieldType>& eb)
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

