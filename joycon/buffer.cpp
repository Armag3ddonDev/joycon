#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

#include "buffer.h"
#include "rumble.h"

/* ---- INPUT BUFFER ---- */

void InputBuffer::clean() {
	std::fill(buf.begin(), buf.end(), 0);
}

const byte& InputBuffer::get_ID() const {
	return buf[0];
}

const byte& InputBuffer::get_timer() const {
	return buf[1];
}

POWER InputBuffer::get_battery_level() const {

	byte battery_level = buf[2] >> 4;
	if (battery_level == 0) {
		return POWER::EMPTY;
	} else if (battery_level <= 2) {
		return POWER::CRITICAL;
	} else if (battery_level <= 4) {
		return POWER::LOW;
	} else if (battery_level <= 6) {
		return POWER::MEDIUM;
	} else {
		return POWER::FULL;
	}
}

const byte& InputBuffer::get_ACK() const {
	this->check_ID(0x21);
	return buf[13];
}

const byte& InputBuffer::get_subcommandID_reply() const {
	this->check_ID(0x21);
	return buf[14];
}

ByteVector InputBuffer::get_reply_data(std::size_t offset, std::size_t length) const {

	this->check_ID(0x21);

	if (length == 0) {
		length = 35;
	}

	if (offset + length > 35 || offset > 35 || length > 35) {
		throw std::out_of_range("Length is too big!");
	}

	return ByteVector(buf.begin() + 15 + offset, buf.begin() + 15 + offset + length);
}

const byte&  InputBuffer::get_reply_data_at(std::size_t idx) const {

	this->check_ID(0x21);

	if (!(idx < 35)) {
		throw std::out_of_range("Index must be less than 35.");
	}
	return buf[15 + idx];
}

ByteVector InputBuffer::get_MCU_FW_update_report() const {
	this->check_ID(0x23);
	return ByteVector(buf.begin() + 13, buf.begin() + 13 + 37);
}

ByteVector InputBuffer::get_AxisData() const {
	this->check_ID({0x30, 0x31, 0x32, 0x33});
	return ByteVector(buf.begin() + 13, buf.begin() + 13 + 36);
}

ByteVector InputBuffer::get_NFC_IR_input_report() const {
	this->check_ID(0x31);
	if (!this->enabledNFC()) {
		throw std::runtime_error("Wrong buffer size. NFC/IR require buffer of size 361.");
	}
	return ByteVector(buf.begin() + 49, buf.begin() + 49 + 313);
}

void InputBuffer::check_ID(byte valid) const {
	const byte& ID = this->get_ID();
	if (ID != valid) {
		std::ostringstream error;
		error << "Wrong mode! ID should be " << std::hex << static_cast<unsigned int>(valid) 
			<< ", but ID is " << std::hex << static_cast<unsigned int>(ID) << std::endl;
		throw std::runtime_error(error.str());
	}
}

void InputBuffer::check_ID(std::unordered_set<byte> valid_list) const {
	const byte& ID = this->get_ID();
	if (valid_list.find(ID) == valid_list.end()) {
		std::ostringstream error;
		error << "Wrong mode! ID should be in {";
		std::string del = "";
		for (auto valid : valid_list) {
			error << del << std::hex << static_cast<unsigned int>(valid);
			del = ", ";
		}
		error << "}, but ID is " << std::hex << static_cast<unsigned int>(ID) << std::endl;
		throw std::runtime_error(error.str());
	}
}

/* ---- OUTPUT BUFFER --- */

OutputBuffer::OutputBuffer(std::size_t dataSize) : BufferBase((11 + dataSize < 11)? throw std::bad_alloc() : 11+dataSize) {

	this->set_rumble_left(Rumble());
	this->set_rumble_right(Rumble());
}

void OutputBuffer::set_cmd(byte in) {
	buf[0] = in;
}

void OutputBuffer::set_GP(byte in) {
	buf[1] = in;
}

void OutputBuffer::set_subcmd(byte in) {
	buf[10] = in;
}

void OutputBuffer::set_rumble_left(const Rumble& rumble) {
	buf[2] = rumble.byte_at(0);
	buf[3] = rumble.byte_at(1);
	buf[4] = rumble.byte_at(2);
	buf[5] = rumble.byte_at(3);
}

void OutputBuffer::set_rumble_right(const Rumble& rumble) {
	buf[6] = rumble.byte_at(0);
	buf[7] = rumble.byte_at(1);
	buf[8] = rumble.byte_at(2);
	buf[9] = rumble.byte_at(3);
}

void OutputBuffer::set_data(const ByteVector& data) {

	if (data.size() + 11 != buf.size()) {
		throw std::length_error("Size mismatch. Data does not fit in the OutputBuffer.");
	}

	std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
}


