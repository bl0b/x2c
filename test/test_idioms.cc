#include "catch.hpp"

#include "x2c.h"

using namespace x2c;

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

TEST_CASE( "idiom 4.1", "ignoring an attribute" )
{
    struct Pouet {
        int i;
    };

    DTD_START(test, pouet, Pouet)
        pouet = A("nothing", &ignore::entity) & A("i", &Pouet::i);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<pouet nothing="" i="1234" />)";
    Pouet* output = NULL;
    REQUIRE_NOTHROW(output = test.parse(iss));
    REQUIRE(output != NULL);
    REQUIRE(output->i == 1234);
    delete output;
}

TEST_CASE( "idiom 4.2", "ignoring an element" )
{
    struct Pouet {
        int i;
    };

    DTD_START(test, pouet, Pouet)
        ELEMENT(nil, int);
        nil = chardata();
        pouet = A("i", &Pouet::i) & E(nil, &ignore::entity);
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<pouet i="1234"><nil>1234</nil></pouet>)";
    Pouet* output = NULL;
    REQUIRE_NOTHROW(output = test.parse(iss));
    REQUIRE(output != NULL);
    REQUIRE(output->i == 1234);
    delete output;
}

