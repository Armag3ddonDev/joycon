#include <iostream>
#include <stdint.h> //for size_t SIZE_MAX macro

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "buffer.h"
#include "rumble.h"

namespace {

//Testing construction of output buffer

//default size should be calling OutputBufer(0), and thus have the size 11
TEST(OutputBufferCTor, TestDefaultCTor) {
	OutputBuffer buf_out;
	EXPECT_EQ(buf_out.size(), 11);
}

//construct with datasize = 0, should init underlaying vector with 11 + 0 as size
TEST(OutputBufferCTor, TestZeroCTor) {
	OutputBuffer buf_out(0);
	EXPECT_EQ(buf_out.size(), 11);
}

//construct with datasize = 2, should init underlaying vector with 11 + 2 as size
TEST(OutputBufferCTor, TestNormalCTor) {
	OutputBuffer buf_out(2);
	EXPECT_EQ(buf_out.size(), 11 + 2);
}

//construct with size = SIZE_MAX to trigger overflow
//Our exception from BufferBase should be thrown
TEST(OutputBufferCTor, TestOverflowCTor) {
	EXPECT_THROW({OutputBuffer buf_out(SIZE_MAX);}, std::bad_alloc);
}

//construct with size = -1 to trigger overflow (usinged int)
//Our exception from BufferBase should be thrown
TEST(OutputBufferCTor, TestUnderflowCTor) {
	EXPECT_THROW({OutputBuffer buf_out(-1);}, std::bad_alloc);
}

//construct with size = SIZE_MAX -12 to trigger overflow
//Exception from std::vector should be thrown
//This Test will fail, if you should have enough space, to create a vector with 2^64 - 2 elements
//which is very unlikely (you need just about 18 petabyte), thats why we expect a throw here
TEST(OutputBufferCTor, TestMaxSizeCTor) {
	EXPECT_THROW({OutputBuffer buf_out(SIZE_MAX-12);}, std::bad_alloc);
}

//Testing Member functions of OutputBuffer

//calling default ctor should initialize the data with 0x00 as cmd
//00 as timing byte
//rumble set to neutral (00 01 40 40) for both joycons
//and subcommand set to 00 (no subcommand sent) and therefore no data afterwards
//The vector in should look like this:
//00 00 00 01 40 40 00 01 40 40 00
//cmd | timing byte | 4 byte with rumble set to neutral for left joycon | 4 byte with rumble set to neutral for right joycon | subcommand id
TEST(OutputBufferMember, TestDefaultContent) {
	OutputBuffer buf_out;
	std::string hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " "); //call to_hex_string to get formated output
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 00"); //and compare with expected result
}

//init outputbuffer with length = 65 and check if data block (after 11th byte) is empty (all 00)
TEST(OutputBufferMember, TestDefaultDataContent)  {
	OutputBuffer buf_out(65);
	std::string hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
					  "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
					  "00 00 00 00 00 00 00 00 00 00 00 00 00 00");
}

//check set_cmd(unsigned char) function
TEST(OutputBufferMember, TestSet_cmd) {
	OutputBuffer buf_out;
	buf_out.set_cmd(0x30);
	std::string hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "30 00 00 01 40 40 00 01 40 40 00");

	buf_out.set_cmd(-1); //unsigned char -> -1 == ff
	hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "ff 00 00 01 40 40 00 01 40 40 00");
}


//create OutputBuffer with multible byte as dataSize and set it with set_dats
//try to set data, with default initialized OutputBuffer (It has no space left for data)
//std::length_error should be thrown from set_data function
//try to set data, with realistic datasize and check for bound
TEST(OutputBufferMember, TestData) {
	OutputBuffer buf_out_empty;
	EXPECT_THROW({buf_out_empty.set_data({0x11, 0x22, 0x33});}, std::length_error); //no space for data

	OutputBuffer buf_out(1);
	EXPECT_NO_THROW(buf_out.set_data({0x11}););
	std::string hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 00 11"); //the last byte should be the data byte which we should be set to 0x11

	buf_out.set_subcmd(0x3F);
	hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 3f 11"); //second last byte (index: 10) should be 3f

	buf_out.set_subcmd(-1);
	hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 ff 11"); //second last byte (index: 10) should be ff

	OutputBuffer buf_real(5);
	buf_real.set_data({0xff, 0xff, 17, 0x3f, 0x3f});
	hexstr = to_hex_string(static_cast<ByteVector>(buf_real), "", " ");
	EXPECT_EQ(hexstr, "00 00 00 01 40 40 00 01 40 40 00 ff ff 11 3f 3f"); //data block should contain ff, ff, 11, 3f, 3f
}

//should change timing byte (index: 1)
TEST(OutputBufferMember, TestSet_GP) {
	OutputBuffer buf_out;
	buf_out.set_GP(1);
	std::string hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 01 00 01 40 40 00 01 40 40 00");

	buf_out.set_GP(-1);
	hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 ff 00 01 40 40 00 01 40 40 00");

	buf_out.set_GP(0x30);
	hexstr = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");
	EXPECT_EQ(hexstr, "00 30 00 01 40 40 00 01 40 40 00");
}

TEST(OutputBufferMember, TestSet_rumble_leftandright) {
	OutputBuffer buf_out;

	Rumble rmbl_left(100, 1); //100Hz with max amplitude
	//100Hz with 1.0 as amplitude => rmbl.data: 0x28 0xc8 0x2a 0x72
	Rumble rmbl_right(200, 0.5); //200Hz with half amplitude
	//200Hz with 0.5 as amplitude => rmbl.data: 0xa8 0x88 0x4a 0x62

	//content of "empty", default set, OutputBuffer:
	std::string expected_empty_buf = "00 00 00 01 40 40 00 01 40 40 00"; //00 01 40 40 - Rumble data with amplitude = 0 -> dont rumble
	std::string test_empty_buf = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");

	EXPECT_EQ(expected_empty_buf, test_empty_buf);


	buf_out.set_rumble_left(rmbl_left);
	std::string expected_left_rumble_buf = "00 00 28 c8 2a 72 00 01 40 40 00";
	std::string test_left_rumble_buf = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");

	EXPECT_EQ(expected_left_rumble_buf, test_left_rumble_buf);


	buf_out.set_rumble_right(rmbl_right);

	std::string expected_right_rumble_buf = "00 00 28 c8 2a 72 a8 88 4a 62 00";
	std::string test_right_rumble_buf = to_hex_string(static_cast<ByteVector>(buf_out), "", " ");

	EXPECT_EQ(expected_right_rumble_buf, test_right_rumble_buf);
}

} //namespace

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

