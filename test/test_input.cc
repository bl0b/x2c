#include "catch.hpp"
#include "x2c.h"

using namespace x2c;

TEST_CASE( "bad input.1", "Malformed XML" )
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

TEST_CASE( "bad input.2", "Malformed XML" )
{
    struct Node {
        Node* left;
        Node* right;
        int value;
        Node() : left(0), right(0), value(0)
        {
            /*std::cerr << "new node " << this << std::endl;*/
        }
        ~Node() {
            /*std::cerr << "delete node l=" << left << " r=" << right << std::endl;*/
            if (left) delete left;
            if (right) delete right;
        }
    };

    DTD_START(tree, node, Node)
        node = A("value", &Node::value) & make_optional(E(node, &Node::left)) & make_optional(E(node, &Node::right));
    DTD_END(tree);

    std::stringstream iss(R"(<node value="1"><node value="0"><node value="2"><node value="1"/<node value="3"/></node></node></node>)");

    Node* output = NULL;
    REQUIRE_THROWS(output = tree.parse(iss));

    REQUIRE(output == (void*)0);
}

