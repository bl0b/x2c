#include "catch.hpp"
#include "x2c.h"
#include <map>

using namespace x2c;

TEST_CASE( "structure 1.1", "Ordered sequence of 2" )
{
    struct Test {
        int a;
        std::string b;
        Test() : a(0), b() {}
    };

    DTD_START(test, x, Test)
        x = A("a", &Test::a) & A("b", &Test::b);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<x a="24" b="totopouet"/>)";
    Test* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 24);
    CHECK(output->b == "totopouet");
    delete output;
}


TEST_CASE( "structure 1.2", "Ordered sequence of 3 (left assoc)" )
{
    struct Test {
        int a;
        std::string b;
        double c;
        Test() : a(0), b() {}
    };

    DTD_START(test, x, Test)
        x = A("a", &Test::a) & A("b", &Test::b) & A("c", &Test::c);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<x a="24" b="totopouet" c="23.6106"/>)";
    Test* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 24);
    CHECK(output->b == "totopouet");
    CHECK(output->c == 23.6106);
    delete output;
}


TEST_CASE( "transient binding", "Inner element edits the same data as its parent element" )
{
    struct Test {
        int a;
        std::string b;
        double c;
        Test() : a(0), b() {}
    };

    DTD_START(test, x, Test)
        ELEMENT(trans, Test);
        x = E(trans);
        trans = A("a", &Test::a) & A("b", &Test::b) & A("c", &Test::c);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<x><trans a="24" b="totopouet" c="23.6106"/></x>)";
    Test* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 24);
    CHECK(output->b == "totopouet");
    CHECK(output->c == 23.6106);
    delete output;
}



struct Foo {
    int a;
    double b;
};


struct Manip {
    double factor;
    Manip() : factor(1.) {}
    void operator () (Foo& f)
    {
        DEBUG_LOG("factor is " << factor << std::endl);
        f.b *= factor;
    }
};

struct Init {
    void operator () (Foo& f)
    {
        f.a = 6106;
        f.b = 101.505;
    }
};

DTD_START(foodtd, foo, Foo)
    ELEMENT(a, int);
    ELEMENT(b, double);
    ELEMENT(factor, double);
    ELEMENT(op, Manip);
    ELEMENT(init, Init);

    a = chardata();
    b = A("value");
    factor = chardata();
    op = E(factor, &Manip::factor);
    foo = E(init) | ((E(a, &Foo::a), E(b, &Foo::b)) & make_optional(E(op)));
DTD_END(foodtd);

const char* XML1 = "<foo><a>1234</a><b value=\"43.21\"/></foo>";
const char* XML2 = "<foo><a>1234</a><b value=\"43.21\"/><op><factor>100</factor></op></foo>";
const char* XML3 = "<foo><b value=\"43.21\"/><a>1234</a><op><factor>100</factor></op></foo>";
const char* XML4 = "<foo><init/></foo>";

TEST_CASE( "complex structure.1", "bigger XML example" )
{

    std::stringstream iss(XML1);
    Foo* output = foodtd.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 1234);
    CHECK(output->b == 43.21);
    delete output;
}

TEST_CASE( "complex structure.2", "bigger XML example" )
{

    std::stringstream iss(XML2);
    Foo* output = foodtd.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 1234);
    CHECK(output->b == 4321.);
    delete output;
}

TEST_CASE( "complex structure.3", "bigger XML example" )
{

    std::stringstream iss(XML3);
    Foo* output = foodtd.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 1234);
    CHECK(output->b == 4321.);
    delete output;
}

TEST_CASE( "complex structure.4", "bigger XML example" )
{

    std::stringstream iss(XML4);
    Foo* output = foodtd.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->a == 6106);
    CHECK(output->b == 101.505);
    delete output;
}

TEST_CASE( "container class binding.1", "push into vector" )
{
    struct X {
        std::vector<int> v;
    };
    DTD_START(vec, x, X)
        ELEMENT(item, int);
        item = A("value");
        x = E(item, &X::v);
    DTD_END(vec);

    std::stringstream iss(R"(<x><item value="23"/><item value="42"/><item value="6106"/></x>)");
    X* output = vec.parse(iss);
    REQUIRE(output != (void*)0);
    REQUIRE(output->v.size() == 3);
    CHECK(output->v[0] == 23);
    CHECK(output->v[1] == 42);
    CHECK(output->v[2] == 6106);
    delete output;
}

TEST_CASE( "container class binding.2", "push into map" )
{
    struct X {
        std::map<std::string, int> v;
        typedef typename unconst_value_type<std::map<std::string, int>::value_type>::type value_type; /* tricky, eh */
    };
    DTD_START(vec, x, X)
        ELEMENT(item, X::value_type);
        item = A("key", &X::value_type::first) & A("value", &X::value_type::second);
        x = E(item, &X::v);
    DTD_END(vec);

    std::stringstream iss(R"(<x><item key="a" value="23"/><item key="b" value="42"/><item key="c" value="6106"/></x>)");
    X* output = vec.parse(iss);
    REQUIRE(output != (void*)0);
    REQUIRE(output->v.size() == 3);
    CHECK(output->v["a"] == 23);
    CHECK(output->v["b"] == 42);
    CHECK(output->v["c"] == 6106);
    delete output;
}


TEST_CASE( "container class binding.3", "push into map (second version)" )
{
    struct X {
        std::map<std::string, int> v;
        typedef std::pair<std::string, int> value_type; /* much simpler to write this way */
    };
    DTD_START(vec, x, X)
        ELEMENT(item, X::value_type);
        item = A("key", &X::value_type::first) & A("value", &X::value_type::second);
        x = E(item, &X::v);
    DTD_END(vec);

    std::stringstream iss(R"(<x><item key="a" value="23"/><item key="b" value="42"/><item key="c" value="6106"/></x>)");
    X* output = vec.parse(iss);
    REQUIRE(output != (void*)0);
    REQUIRE(output->v.size() == 3);
    CHECK(output->v["a"] == 23);
    CHECK(output->v["b"] == 42);
    CHECK(output->v["c"] == 6106);
    delete output;
}

TEST_CASE( "recursion.1", "recursive container" )
{
    struct Node {
        Node* left;
        Node* right;
        int value;
        Node() : left(0), right(0), value(0) {}
        ~Node() { if (left) delete left; if (right) delete right; }
    };

    DTD_START(tree, node, Node)
        node = A("value", &Node::value) & make_optional(E(node, &Node::left)) & make_optional(E(node, &Node::right));
    DTD_END(tree);

    std::stringstream iss(R"(<node value="2"><node value="1"/><node value="3"/></node>)");

    Node* output = tree.parse(iss);

    REQUIRE(output != (void*)0);
    CHECK(output->value == 2);
    REQUIRE(output->left != (void*)0);
    CHECK(output->left->value == 1);
    CHECK(output->left->left == (void*)0);
    CHECK(output->left->right == (void*)0);
    REQUIRE(output->right != (void*)0);
    CHECK(output->right->value == 3);
    CHECK(output->right->left == (void*)0);
    CHECK(output->right->right == (void*)0);
    delete output;
}

