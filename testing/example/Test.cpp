#include "testing/Test.h"

TEST(example1)
{
#pragma warning(disable:4127)
	CHECK(true);
	CHECK(false);
#pragma warning(default:4127)
}

bool f()
{
	throw std::exception();
}

TEST(example2)
{
	CHECK(f());
}

TEST(example3)
{
	EQUALS(4, 4);
	EQUALS(4, 5);
}

TEST(example4)
{
	EQUALS(true, f());
}

TEST(example5)
{
	throw std::exception();
}
