#include "structure.h"

#include "XML.h"

/*#include <tuple>*/
#include <iostream>
#include <sstream>

#include <typeinfo>

#if 0

template <int I, int TSize, typename Tuple>
struct iterate_over_tuple_impl
    : public iterate_over_tuple_impl<I + 1, TSize, Tuple>
{
    template <typename Function>
        void operator () (Function& f, Tuple& t)
        {
            f(std::get<I>(t));
            iterate_over_tuple_impl<I + 1, TSize, Tuple>::operator () (f, t);
        }
};

template <int I, typename Tuple>
struct iterate_over_tuple_impl<I, I, Tuple> {
    template <typename Function>
        void operator () (Function& f, Tuple& t) {}
};

template <typename Function, typename... Args>
void iterate_over_tuple(Function& f, std::tuple<Args...>& t)
{
    iterate_over_tuple_impl<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}

#endif

struct toto {
    std::string sep;
    std::string sep2;
    std::stringstream oss;
    toto(const char* s, const char* s2) : sep(s), sep2(s2), oss() {}
    template <typename kls, typename... Elements>
        void operator () (combination<kls, Elements...>& x)
        {
            oss << sep << x;
            sep = sep2;
        }

    template <typename X>
        void operator () (X x) 
        {
            oss << sep << x;
            sep = sep2;
        }
};

template <typename... Elements>
std::ostream& operator << (std::ostream& os, const combination<ordered_sequence, Elements...>& seq)
{
    toto f { "", " " };
    iterate_over_tuple(f, seq);
    os << f.oss.str();
    return os;
}

template <typename... Elements>
std::ostream& operator << (std::ostream& os, const combination<alternative, Elements...>& seq)
{
    toto f { "(", " | " };
    iterate_over_tuple(f, seq);
    os << f.oss.str() << ')';
    return os;
}

template <typename... Elements>
std::ostream& operator << (std::ostream& os, const combination<unordered_sequence, Elements...>& seq)
{
    toto f { "{", ", " };
    iterate_over_tuple(f, seq);
    os << f.oss.str() << '}';
    return os;
}

template <typename Element>
std::ostream& operator << (std::ostream& os, const combination<single, Element>& x)
{
    toto f { "S[", "" };
    iterate_over_tuple(f, x);
    return os << f.oss.str() << "]";
}

template <typename Element>
std::ostream& operator << (std::ostream& os, const combination<multiple, Element>& x)
{
    toto f { "M[", "" };
    iterate_over_tuple(f, x);
    return os << f.oss.str() << "]";
}

template <typename Element>
std::ostream& operator << (std::ostream& os, const combination<optional<single>, Element>& x)
{
    toto f { "?S[", "" };
    iterate_over_tuple(f, x);
    return os << f.oss.str() << "]?";
}

template <typename Element>
std::ostream& operator << (std::ostream& os, const combination<optional<multiple>, Element>& x)
{
    toto f { "?M[", "" };
    iterate_over_tuple(f, x);
    return os << f.oss.str() << "]?";
}


#if 0
struct transf {
    template <typename Element>
        static std::string && transform_impl(Element & e)
        {
            std::stringstream oss;
            oss << '"' << e << '"';
            return std::move(oss.str());
        }

    static std::string transform(int x) { return std::move(transform_impl(x)); }
    static std::string transform(double x) { return std::move(transform_impl(x)); }
    static std::string transform(std::string& x) { return std::move(transform_impl(x)); }
    static std::string transform(std::string&& x) { return std::move(transform_impl(x)); }
};
#endif

struct empty {};

struct ctest : public std::tuple<int, int>, public empty {};

struct id {
    struct pouet {
        pouet(int) {}
        pouet(double) {}
        pouet(const std::string&) {}
        template <typename S, typename F> pouet(target_descr<S, F>& x) {}
    };
    template <typename X> static pouet && transform(X x) { return std::move(pouet(x)); }
};

std::ostream& operator << (std::ostream& os, const id::pouet& x) { return os << "pouet"; }

template <typename EXPR>
void test_operators(const EXPR& e)
{
    std::cout << e << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    auto a = transformer<id>::transform(e);
    std::cout << a << std::endl << std::endl;
}

template <typename S, typename F>
std::ostream& operator << (std::ostream& o, attribute_target_descr<S, F>& a)
{
    o << "attribute\"";
    o << a.name;
    o << "\"->";
    o << typeid(F).name();
    o << '(';
    o << a.field;
    o << ')';
    return o;
}

template <typename S, typename F>
std::ostream& operator << (std::ostream& o, element_target_descr<S, F>& a)
{
    o << "element\"";
    o << a.name;
    o << "\"->";
    o << typeid(F).name();
    o << '(';
    o << a.field;
    o << ')';
    return o;
}


int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    auto a = make_single(1);
    auto b = make_single(42.23);
    auto c = make_single(std::string("totopouet"));

    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(a); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(b); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(c); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(a & b); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(a | b); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators((a, b)); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(a & b & c); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(make_single(42)); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(make_single(42) & make_single(std::string("pouet"))); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(make_single(42) & (make_single(std::string("pouet")), make_multiple(123.45))); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(make_single(42) & (make_single(std::string("pouet")), a | make_optional(make_multiple(123.45)))); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(a); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(b); }
    std::cout << "=========================================================" << std::endl;
    { DEBUG; test_operators(c); }
    std::cout << "=========================================================" << std::endl;

    return 0;
}

