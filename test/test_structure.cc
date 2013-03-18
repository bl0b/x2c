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

