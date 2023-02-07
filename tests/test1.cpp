#include "asimpletest/lib.h"
#include "testing/Test.h"

CATCH_TEST_CASE("add_zeroes_test__", "[test1]")
{
	auto b = add_two_numbers(0, 0);
	EQUALS(b, 0);
}

CATCH_TEST_CASE("add_zero_test__", "[test1]")
{
	auto b = add_two_numbers(5, 0);
	EQUALS(b, 5);
}

CATCH_TEST_CASE("add_arbitrary_test__", "[test1]")
{
	auto b = add_two_numbers(5, 1);
	EQUALS(b, 6);
}
