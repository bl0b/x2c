#include "catch.hpp"

#include "x2c.h"

using namespace x2c;

bool pred(int* x) { return *x > 10 && *x < 30; };

TEST_CASE( "predicates.1", "validating an attribute value" )
{
    struct Test {
        int x;
    };
    DTD_START(test, i, Test)
        i = A("value", &Test::x) / pred;
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i value="42"></i>)";
    Test* output = NULL;
    REQUIRE_THROWS(output = test.parse(iss));
    REQUIRE(output == (void*)0);
    delete output;
}

TEST_CASE( "predicates.2", "validating an attribute value" )
{
    struct Test {
        int x;
    };
    DTD_START(test, i, Test)
        i = A("value", &Test::x) / pred;
    DTD_END(test);

    std::stringstream iss;
    iss << R"(<i value="23"></i>)";
    Test* output;
    REQUIRE_NOTHROW(output = test.parse(iss));
    REQUIRE(output != (void*)0);
    REQUIRE(output->x == 23);
    delete output;
}


bool switch_is_on(std::string* x)
{
    std::cerr << "switch ? " << (*x) << std::endl;
    return *x == "on";
}
bool switch_is_off(std::string* x)
{
    std::cerr << "switch ? " << (*x) << std::endl;
    return *x == "off";
}

struct Test {
    std::string a;
    int b;
};

DTD_START(test, i, Test)
    i = (A("switch", &Test::a) / switch_is_on & A("value", &Test::b))
      | (A("switch", &Test::a) / switch_is_off & A("foobar", &Test::b));
DTD_END(test);

TEST_CASE( "predicates.3.1", "structure conditioned to an attribute value" )
{
    std::stringstream iss;
    iss << R"(<i switch="on" value="23"/>)";
    Test* output = NULL;
    REQUIRE_NOTHROW(output = test.parse(iss));
    REQUIRE(output != NULL);
    REQUIRE(output->a == "on");
    REQUIRE(output->b == 23);
    delete output;
}

TEST_CASE( "predicates.3.2", "structure conditioned to an attribute value" )
{
    std::stringstream iss;
    iss << R"(<i switch="off" value="23"/>)";
    Test* output = NULL;
    REQUIRE_THROWS(output = test.parse(iss));
    REQUIRE(output == NULL);
}

TEST_CASE( "predicates.3.3", "structure conditioned to an attribute value" )
{
    std::stringstream iss;
    iss << R"(<i switch="on" foobar="23"/>)";
    Test* output = NULL;
    REQUIRE_THROWS(output = test.parse(iss));
    REQUIRE(output == NULL);
}

TEST_CASE( "predicates.3.4", "structure conditioned to an attribute value" )
{
    std::stringstream iss;
    iss << R"(<i switch="off" foobar="23"/>)";
    Test* output = NULL;
    REQUIRE_NOTHROW(output = test.parse(iss));
    REQUIRE(output != NULL);
    REQUIRE(output->a == "off");
    REQUIRE(output->b == 23);
    delete output;
}

