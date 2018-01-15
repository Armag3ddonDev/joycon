#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../../buffer.h"

namespace {


TEST(OutputBuffer, TestOutputBuffer) {
	OutputBuffer buf_out;
}

}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

