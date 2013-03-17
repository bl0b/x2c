#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "XML.h"

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
}


TEST_CASE( "idiom 3", "Element modification from an inner element" )
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
}


struct Mod {
    int init;
    Mod() : init(0) {}
    void operator () (int& x) { x = 1234; }
};

TEST_CASE( "idiom 3.1", "Element modification from an inner element with inner structure" )
{
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
}


TEST_CASE( "bad input", "Malformed XML" )
{
    std::string expect = "Error at line 1 column 13";

    DTD_START(test, i, int)
        i = A("value");
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i value="42"</i>)";
    int* output;
    REQUIRE_THROWS(output = test.parse(iss));
    /*try {*/
        /*int* output = test.parse(iss);*/
    /*} catch (xml_exception& e) {*/
        /*REQUIRE(e.what() == expect);*/
    /*}*/
}

