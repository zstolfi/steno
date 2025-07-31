#include <gtest/gtest.h>

TEST(HelloTest, FirstTest) {
	int i = 1;
	EXPECT_TRUE(i);
	EXPECT_EQ(i, 1);
}
