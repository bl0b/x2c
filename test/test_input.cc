#include "catch.hpp"
#include "XML.h"

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

