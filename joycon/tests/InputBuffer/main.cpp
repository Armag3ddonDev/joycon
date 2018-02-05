#include <iostream>
#include <stdint.h> //for size_t SIZE_MAX macro

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "buffer.h"

namespace {

//Testing construction of input buffer

//Default CTor has bEnabledNFC set to false, which calls BufferBase with size 50
//thus InputBuffer should have the size: 50
TEST(InputBufferCTor, TestDefaultCTor) {
	InputBuffer buf_in;
	EXPECT_EQ(buf_in.size(), 50);

	std::vector<unsigned char> test(50); //copy all elements of buf_in into new vector
	for(unsigned int i = 0; i < 50; ++i)
		test[i] = buf_in.data()[i];

	std::vector<unsigned char> empty(50, 0); //we expect a 50 long array with 0

	EXPECT_EQ(test, empty); //compare content of both vectors
}

//bEnabledNFC is set to true, so Buffer should have the size: 362
TEST(InputBufferCTor, TestNFCCTor) {
	InputBuffer buf_in(true);
	EXPECT_EQ(buf_in.size(), 362);

	std::vector<unsigned char> test(362); //copy all elements of buf_in into new vector
	for(unsigned int i = 0; i < 362; ++i)
		test[i] = buf_in.data()[i];

	std::vector<unsigned char> empty(362, 0); //we expect a 362 long array with 0

	EXPECT_EQ(test, empty); //compare content of both vectors
}

//Testing member of input buffer

//Write directly to underlying vector and check if the data is saved correctly
TEST(InputBufferMember, Testdata) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	std::vector<unsigned char> test(50, 0);
	for(unsigned char i = 0; i < 50; ++i) {
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
		test[i] = i;
	}

	//read out data
	std::vector<unsigned char> content(50);
	for(unsigned int i = 0; i < 50; ++i)
		content[i] = buf_in.data()[i];

	EXPECT_EQ(test, content);
}

//Check if command byte is read out correctly
TEST(InputBufferMember, Testget_ID) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	EXPECT_EQ(buf_in.get_ID(), 0x00); //first element, cmd byte, should be 0x00
}

//Check if ACK byte is read out correctly
TEST(InputBufferMember, Testget_ACK) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//ACK byte is only set and used in command ID = 0x21
	//therefore, cmd id is not 0x21 get_ACK should throw
	EXPECT_THROW({buf_in.get_ACK();}, std::runtime_error);


	//In Production, only hidapi should write to InputBuffer. Therefore we only
	//exposed BufferInput::data() as writeable (unsigned char*) for the c-api
	//You, the user should only extract the necessary information through our
	//InputBuffer Interface. If you want to modify the data and pass it somewhere
	//else, create a copy/built your own Buffer/Data structure.
	//If you still modify the buffer, prepare to catch our exceptions

	//set cmd id to 0x21, for testing!
	buf_in.data()[0] = 0x21; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	EXPECT_EQ(buf_in.get_ACK(), 0xD); //ack byte is at index 13 and should contain 0xD
}

//Check if subcommand id byte is read out correctly
TEST(InputBufferMember, Testget_subcommandID_reply) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//subcommandID byte is only set and used in command ID = 0x21
	//therefore, cmd id is not 0x21 get_subcommandID_reply should throw
	EXPECT_THROW({buf_in.get_subcommandID_reply();}, std::runtime_error);

	//set cmd id to 0x21, for testing!
	buf_in.data()[0] = 0x21; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	EXPECT_EQ(buf_in.get_subcommandID_reply(), 0xE); //subcommand id byte is at index 14 and should contain 0xE
}

//Check if reply data is read out correctly
TEST(InputBufferMember, Testget_reply_data) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//reply data is only set and used in command ID = 0x21
	//therefore, cmd id is not 0x21 get_replay_data should throw
	EXPECT_THROW({buf_in.get_reply_data();}, std::runtime_error);

	//set cmd id to 0x21, for testing!
	buf_in.data()[0] = 0x21; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	ByteVector reply_data = buf_in.get_reply_data();

	//calling reply data with no arguments, should read out the full reply data block
	//which is 35bytes long
	EXPECT_EQ(reply_data.size(), 35);

	//reply data block starts at index 15 -> 0x0f
	//therefore it should contain {0x0f, 0x10, 0x11, ..., 0x31}
	std::vector<unsigned char> test(35);
	std::vector<unsigned char> expected(35);
	for(unsigned char i = 0; i < 35; ++i) {
		test[i] = i + 0x0f;
		expected[i] = reply_data[i];
	}

	EXPECT_EQ(test, expected);

	//we can also read out only parts of the reply data block by specifying and offset
	//and a length to read

	//out of range checks
	EXPECT_THROW({buf_in.get_reply_data(-1, 0);}, std::out_of_range); //offset overflow
	EXPECT_THROW({buf_in.get_reply_data(-1, -1);}, std::out_of_range); //offset & length overflow
	EXPECT_THROW({buf_in.get_reply_data(-1, 36);}, std::out_of_range); //offset overflow & length to big
	EXPECT_THROW({buf_in.get_reply_data(-17, 15);}, std::out_of_range); //offset overflow & length in range
	EXPECT_THROW({buf_in.get_reply_data(15, 21);}, std::out_of_range); //offset in range, length in range, but offset + length to big

	ByteVector reply_data1 = buf_in.get_reply_data(10, 5);

	//reply data block starts at index 15 + offset 10 -> 0x19
	//therefore it should contain {0x19, 0x20, 0x21, 0x22, 0x23}
	std::vector<unsigned char> test1;
	std::vector<unsigned char> expected1;
	test1.reserve(5);
	expected1.reserve(5);
	for(unsigned char i = 0; i < 5; ++i) {
		test1.push_back(i + 0x0f + 10);
		expected1.push_back(expected[i+10]);
	}

	EXPECT_EQ(test1, expected1);
}

//Check if reply data at is read out correctly
TEST(InputBufferMember, Testget_replay_data_at) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//reply data is only set and used in command ID = 0x21
	//therefore, cmd id is not 0x21 get_replay_data should throw
	EXPECT_THROW({buf_in.get_reply_data();}, std::runtime_error);

	//set cmd id to 0x21, for testing!
	buf_in.data()[0] = 0x21; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//out of range checks
	EXPECT_THROW({buf_in.get_reply_data_at(-1);}, std::out_of_range);
	EXPECT_THROW({buf_in.get_reply_data_at(36);}, std::out_of_range);

	//realistiic calls
	EXPECT_EQ(buf_in.get_reply_data_at(19), 0x22);
	EXPECT_EQ(buf_in.get_reply_data_at(1), 0x10);
	EXPECT_EQ(buf_in.get_reply_data_at(0), 0x0f);
}

//Check if MCU FW update report is read out correctly
TEST(InputBufferMember, Testget_MCU_FW_update_report) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//MCU FW update report is only set and used in command ID = 0x23
	//therefore, cmd id is not 0x23 get_replay_data should throw
	EXPECT_THROW({buf_in.get_MCU_FW_update_report();}, std::runtime_error);

	//set cmd id to 0x23, for testing!
	buf_in.data()[0] = 0x23; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//compare content
	//should start from 0x0d until 0x31
	ByteVector result = buf_in.get_MCU_FW_update_report();

	std::vector<unsigned char> expected(37);
	std::vector<unsigned char> test(37);
	for(unsigned char i = 0; i < 37; ++i) {
		expected[i] = i + 0x0d;
		test[i] = result[i];
	}

	EXPECT_EQ(expected, test);
}

//Check if MCU FW update report is read out correctly
TEST(InputBufferMember, Testget_AxisData) {
	InputBuffer buf_in;

	//fill buffer with data
	//from 0x00 until 0x32 (50 elements) to check if proper element is read out
	for(unsigned char i = 0; i < 50; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//Axis Data is set and used following command IDs = {0x30, 0x31, 0x32, 0x33}
	//therefore, cmd id is not {0x30, 0x31, 0x32, 0x33} get_replay_data should throw
	EXPECT_THROW({buf_in.get_AxisData();}, std::runtime_error);

	//test different command ids
	buf_in.data()[0] = 0x23; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
	EXPECT_THROW({buf_in.get_AxisData();}, std::runtime_error); //0x23 not supported
	//supported ones
	buf_in.data()[0] = 0x30; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
	EXPECT_NO_THROW({buf_in.get_AxisData();}); //0x30 supported
	buf_in.data()[0] = 0x31; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
	EXPECT_NO_THROW({buf_in.get_AxisData();}); //0x31 supported
	buf_in.data()[0] = 0x32; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
	EXPECT_NO_THROW({buf_in.get_AxisData();}); //0x32 supported
	buf_in.data()[0] = 0x33; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!
	EXPECT_NO_THROW({buf_in.get_AxisData();}); //0x33 supported

	ByteVector result = buf_in.get_AxisData();

	//compare content
	//should start from 0x0d until 0x30
	std::vector<unsigned char> expected(36);
	std::vector<unsigned char> test(36);
	for(unsigned char i = 0; i < 36; ++i) {
		expected[i] = i + 0x0d;
		test[i] = result[i];
	}

	EXPECT_EQ(expected, test);
}

//Check if NFC/IR input is read out correctly
TEST(InputBufferMember, Testget_NFC_IR_input_report) {
	InputBuffer buf_in(true); //need to set to true, to get NFC/IR data

	//fill buffer with data
	//0-49 header + 313 byte NFC/IR data
	//going from 0x00 - 0xFF + missing with overflow in unsigned char -> 0x00 .. 0xFF, 0x00 .. 0x69
	//(313 elements) to check if proper element is read out
	for(unsigned int i = 0; i < 362; ++i)
		buf_in.data()[i] = i; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	//NFC/IR Data is set and used only in command ID = 0x23
	//therefore, cmd id is not 0x23 and get_NFC_IR_input_report should throw
	EXPECT_THROW({buf_in.get_NFC_IR_input_report();}, std::runtime_error);

	//set cmd id to 0x31, for testing!
	buf_in.data()[0] = 0x31; //DONT CHANGE INPUT BUFFER IN PRODUCTION LIKE THIS!

	ByteVector result = buf_in.get_NFC_IR_input_report();

	//compare content
	//should start from 0x31 until 0x69 (with one overflow 0xff, 0x00 in the middle)
	//resulting in 313 bytes
	std::vector<unsigned char> expected(313);
	std::vector<unsigned char> test(313);
	for(unsigned int i = 0; i < 313; ++i) {
		expected[i] = i + 0x31;
		test[i] = result[i];
	}

	EXPECT_EQ(expected, test);
}


} //namespace

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

