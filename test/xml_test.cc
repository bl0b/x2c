#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "XML.h"

TEST_CASE( "structure operators 1 (ordered)", "Left/Right assoc equivalence" )
{
    auto seq1 = (make_single(1) & make_single(2)) & make_single(3);
    auto seq2 = make_single(1) & (make_single(2) & make_single(3));
    REQUIRE(typeid(seq1) == typeid(seq2));
}


TEST_CASE( "structure operators 1 (alternative)", "Left/Right assoc equivalence" )
{
    auto seq1 = (make_single(1) | make_single(2)) | make_single(3);
    auto seq2 = make_single(1) | (make_single(2) | make_single(3));
    REQUIRE(typeid(seq1) == typeid(seq2));
}


TEST_CASE( "structure operators 1 (unordered)", "Left/Right assoc equivalence" )
{
    auto seq1 = ((make_single(1), make_single(2)), make_single(3));
    auto seq2 = (make_single(1), (make_single(2), make_single(3)));
    REQUIRE(typeid(seq1) == typeid(seq2));
}


TEST_CASE( "idiom 1", "Element evaluation by chardata" )
{
    DTD_START(test, i, int)
        i = chardata();
    DTD_END(test);

    std::stringstream iss;
    iss << "<i>42</i>";
    int* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    REQUIRE(*output == 42);
    delete output;
}


TEST_CASE( "idiom 2", "Element evaluation by single attribute" )
{
    DTD_START(test, i, int)
        i = A("value");
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i value="42"></i>)";
    int* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    REQUIRE(*output == 42);
    delete output;
}


TEST_CASE( "idiom 3.1", "Element modification from an inner element" )
{
    struct Mod { void operator () (int& x) { x = 1234; } };
    DTD_START(test, i, int)
        ELEMENT(mod, Mod);
        i = E(mod);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i><mod/></i>)";
    int* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    REQUIRE(*output == 1234);
    delete output;
}


TEST_CASE( "idiom 3.2", "Element modification from an inner element with inner structure (attribute)" )
{
    struct Mod {
        int init;
        Mod() : init(0) {}
        void operator () (int& x) { x = 1234; }
    };

    DTD_START(test, i, int)
        ELEMENT(mod, Mod);
        mod = A("value", &Mod::init);
        i = E(mod);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i><mod value="1234"/></i>)";
    int* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    REQUIRE(*output == 1234);
    delete output;
}


TEST_CASE( "idiom 3.3", "Element modification from an inner element with inner structure (element)" )
{
    struct Mod {
        int init;
        Mod() : init(0) {}
        void operator () (int& x) { x = 1234; }
    };

    DTD_START(test, i, int)
        ELEMENT(mod, Mod);
        ELEMENT(init, int);
        mod = E(init, &Mod::init);
        init = chardata();
        i = E(mod);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i><mod><init>1234</init></mod></i>)";
    int* output = test.parse(iss);

    REQUIRE(output != (void*)0);
    REQUIRE(*output == 1234);
    delete output;
}


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


TEST_CASE( "bad input", "Malformed XML" )
{
    std::string expect = "Error at line 1 column 13";

    DTD_START(test, i, int)
        i = A("value");
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i value="42"</i>)";
    int* output = NULL;
    REQUIRE_THROWS(output = test.parse(iss));
    REQUIRE(output == NULL);
}

