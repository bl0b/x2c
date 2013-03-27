#include "catch.hpp"
#include "x2c.h"

struct dummy {};

namespace x2c {

template <>
struct data_binder<dummy, dummy, void> {
    std::string name;
    struct {
        iterator_base<dummy>* iterator() const { return NULL; }
    }* elt;

    dummy* install(dummy*) const { return NULL; }
    bool after(dummy*, dummy*) const { return true; }
    void rollback(dummy**) const {}
};

}

using namespace x2c;

data_binder<dummy, dummy, void> B(const std::string& name) { return { name, NULL }; }

template <typename X>
iterator_base<dummy>* I(const X& x) { return make_iterator<dummy>(x); }


TEST_CASE( "iterators behaviour.0", "empty" )
{
    auto i = new iterator_empty<int>();
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    CHECK(!i->accept("foobar"));
    CHECK(!i->consume("foobar", NULL));
    delete i;
}


TEST_CASE( "iterators behaviour.1", "single" )
{
    auto i = I(make_single(B("foo")));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("foo"));
    CHECK(!i->accept("bad"));

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());

    CHECK(!i->consume("anything", NULL));

    CHECK(!i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}


TEST_CASE( "iterators behaviour.2", "multiple" )
{
    auto i = I(make_multiple(B("foo")));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("foo"));
    CHECK(!i->accept("bad"));

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(!i->consume("anything", NULL));

    CHECK(!i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}
TEST_CASE( "iterators behaviour.3", "optional<single>" )
{
    auto i = I(make_optional(make_single(B("foo"))));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("foo"));
    CHECK(!i->accept("bad"));

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());

    CHECK(!i->consume("anything", NULL));

    CHECK(!i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}


TEST_CASE( "iterators behaviour.4", "optional<multiple>" )
{
    auto i = I(make_optional(make_multiple(B("foo"))));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("foo"));
    CHECK(!i->accept("bad"));

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->consume("foo", NULL));

    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(i->is_accepting());

    CHECK(!i->consume("anything", NULL));

    CHECK(!i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}

TEST_CASE( "iterators behaviour.5", "ordered_sequence of singles" )
{
    auto i = I(make_single(B("a")) & make_single(B("b")));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("a"));
    CHECK(!i->accept("b"));

    REQUIRE(i->consume("a", NULL));
    CHECK(i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(!i->accept("a"));
    CHECK(i->accept("b"));
    
    REQUIRE(i->consume("b", NULL));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}

TEST_CASE( "iterators behaviour.6.1", "unordered_sequence of singles" )
{
    auto i = I((make_single(B("a")), make_single(B("b"))));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("a"));
    CHECK(i->accept("b"));

    REQUIRE(i->consume("a", NULL));
    CHECK(i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(!i->accept("a"));
    CHECK(i->accept("b"));
    
    REQUIRE(i->consume("b", NULL));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}

TEST_CASE( "iterators behaviour.6.2", "unordered_sequence of singles" )
{
    auto i = I((make_single(B("a")), make_single(B("b"))));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("a"));
    CHECK(i->accept("b"));

    REQUIRE(i->consume("b", NULL));
    CHECK(i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(!i->accept("b"));
    CHECK(i->accept("a"));
    
    REQUIRE(i->consume("a", NULL));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());
    delete i;
}

TEST_CASE( "iterators behaviour.7.1", "alternative of singles" )
{
    auto i = I((make_single(B("a")) | make_single(B("b"))));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("a"));
    CHECK(i->accept("b"));

    REQUIRE(i->consume("a", NULL));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());

    CHECK(!i->accept("b"));
    CHECK(!i->accept("a"));
    delete i;
}

TEST_CASE( "iterators behaviour.7.2", "alternative of singles" )
{
    auto i = I((make_single(B("a")) | make_single(B("b"))));
    CHECK(!i->is_good());
    CHECK(!i->is_done());
    CHECK(i->is_accepting());

    CHECK(i->accept("a"));
    CHECK(i->accept("b"));

    REQUIRE(i->consume("b", NULL));
    CHECK(i->is_good());
    CHECK(i->is_done());
    CHECK(!i->is_accepting());

    CHECK(!i->accept("b"));
    CHECK(!i->accept("a"));
    delete i;
}

