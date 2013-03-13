#ifndef _XML_DATABINDING_H_
#define _XML_DATABINDING_H_

/* simple setter */
template <typename StrucType, typename FieldType>
struct data_binder<FieldType StrucType::*, Element<StrucType>> {
    typedef Element<FieldType> entity_type;

    std::string name;
    FieldType StrucType::* target;
    const entity_type* elt;

    FieldType* install(StrucType* container) const
    {
        return &container->*target;
    }

    void after(StrucType* ptr, FieldType data) const
    {}
};


/* simple setter with allocator */
template <typename StrucType, typename FieldType>
struct data_binder<FieldType* StrucType::*, Element<StrucType>> {
    typedef Element<FieldType> entity_type;

    std::string name;
    FieldType* StrucType::* target;
    const entity_type* elt;

    FieldType* install(StrucType* container) const
    {
        container->*target = new FieldType();
        return container->*target;
    }

    void after(StrucType* container, FieldType data) const
    {}
};


/* manipulator (target type is not related to element::eval_type) */
template <typename EvalType, typename Manipulator>
struct data_binder<Manipulator, Element<EvalType>> {
    typedef Element<Manipulator> entity_type;

    std::string name;
    const entity_type* elt;

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
struct data_binder<EvalType, Element<EvalType>> {
    typedef Element<EvalType> entity_type;

    std::string name;
    const entity_type* elt;

    EvalType* install(EvalType* container)
    {
        return container;
    }

    void after(EvalType* container, EvalType* data)
    {}
};


/* attribute and chardata eval_to binding */
template <typename EvalType>
struct data_binder<EvalType, std::string> {
    std::string name;
    typedef std::string entity_type;

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
struct data_binder<FieldType StrucType::*, std::string> {
    std::string name;
    FieldType StrucType::* field;
    typedef std::string entity_type;

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


#define CHARDATA_NAME "#CHARDATA"

template <typename IntegralType>
struct resolve_bindings_integral {
    /* manipulator */
    /* transient */
    template <typename ManipulatorOrTransient>
        static
        data_binder<ManipulatorOrTransient, Element<IntegralType>>
        transform(const Element<ManipulatorOrTransient>* e)
        {
            return { e->name, e };
        }

    /* attr eval_to */
    static
    data_binder<IntegralType, std::string>
    transform(const attr_eval & ae)
    {
        return { ae.name };
    }

    /* chardata eval_to */
    static
    data_binder<IntegralType, std::string>
    transform(const CharData &)
    {
        return { CHARDATA_NAME };
    }
};

template <typename StrucType>
struct resolve_bindings_class {
    /* manipulator */
    /* transient */
    template <typename ManipulatorOrTransient>
        static
        data_binder<ManipulatorOrTransient, Element<StrucType>>
        transform(const Element<ManipulatorOrTransient>* e)
        {
            return { e->name, e };
        }

    /* attr eval_to */
    static
    data_binder<StrucType, std::string>
    transform(const attr_eval & ae)
    {
        return { ae.name };
    }

    /* chardata eval_to */
    static
    data_binder<StrucType, std::string>
    transform(const CharData &)
    {
        return { CHARDATA_NAME };
    }


    /* simple setter */
    /* simple setter with allocator */
    template <typename FieldType>
        static
        data_binder<FieldType StrucType::*, Element<StrucType>>
        transform(const elt_binding<StrucType, FieldType>& eb)
        {
            return { eb.elt->name, eb.field, eb.elt };
        }

    /* attr binding */
    template <typename FieldType>
        static
        data_binder<FieldType StrucType::*, std::string>
        transform(const attr_binding<StrucType, FieldType>& ab)
        {
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

