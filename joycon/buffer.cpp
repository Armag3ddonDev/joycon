#include <iostream>
#include <sstream>

#include "buffer.h"

/* ------ BYTE BASE ----- */

unsigned long int ByteBase::to_int(std::size_t start, std::size_t length, bool bigEndian) const {

	if (length + start > this->size()) {
		throw std::out_of_range("Length is too big!");
	}

	return this->to_int(this->begin() + start, length, bigEndian);
}

unsigned long int ByteBase::to_int(const_byte_iterator it_begin, std::size_t length, bool bigEndian = true) const {

	if (sizeof(unsigned long int) < length) {
		throw std::overflow_error("Cant convert to int - length is too big.");
	}

	unsigned long int res{ 0 };

	for (std::size_t i = 0; i < length; ++i) {
		std::size_t idx = (bigEndian) ? i : length - 1 - i;
		res = (res << 8) + *(it_begin + idx);
	}

	return res;
}

std::string ByteBase::to_hex_string(std::size_t start, std::size_t length, std::string prefix = "0x", std::string delimiter = "") const {

	if (length + start > this->size()) {
		throw std::out_of_range("Length is too big!");
	}

	return this->to_hex_string(this->begin() + start, this->begin() + start + length - 1, prefix, delimiter);
}

std::string ByteBase::to_hex_string(const_byte_iterator it_begin, const_byte_iterator it_end, std::string prefix = "0x", std::string delimiter = "") const {

	std::stringstream sstream;
	if (it_begin != it_end) {
		sstream << prefix;
	}

	std::string del = "";

	for (const_byte_iterator it = it_begin; it != it_end; ++it) {

		const unsigned char& current_byte = *it;

		std::string fill = "";
		if (current_byte <= 0x0F) { fill = "0"; }

		sstream << del << fill << std::hex << static_cast<unsigned int>(current_byte);

		del = delimiter;
	}

	return sstream.str();
}

void ByteBase::print(unsigned int size) const {
	if (size > this->size()) {
		throw std::out_of_range("Size is too big!");
	}
	std::cout << this->to_hex_string(this->begin(), this->begin + size, "", " ");
}

void ByteBase::print(const_byte_iterator it_begin, const_byte_iterator it_end) const {

	std::cout << to_hex_string(it_begin, it_end, "", " ") << std::endl;
}

/* ---- INPUT BUFFER ---- */

InputBuffer::InputBuffer(bool bEnabledNFC) : 
	BufferBase(bEnabledNFC ? 362 : 50)
{

}

void InputBuffer::clean() {
	std::fill(buf.begin(), buf.end(), 0);
}

const unsigned char& InputBuffer::get_cmd() const {
	return buf[0];
}

const unsigned char& InputBuffer::get_timer() const {
	return buf[1];
}

POWER InputBuffer::get_battery_level() const {

	unsigned char battery_level = buf[2] & 0xF;
	if (battery_level == 0) {
		return POWER::EMPTY;
	} else if (battery_level < 2) {
		return POWER::CRITICAL;
	} else if (battery_level < 4) {
		return POWER::LOW;
	} else if (battery_level < 6) {
		return POWER::MEDIUM;
	} else {
		return POWER::FULL;
	}
}

const unsigned char& InputBuffer::get_ACK() const {
	return buf[13];
}

const unsigned char& InputBuffer::get_subcommandID_reply() const {
	return buf[14];
}

const ByteSubVector InputBuffer::get_reply_data() const {
	this->check_ID(0x21);
	return ByteSubVector(buf.begin() + 15, buf.begin() + 49 + 1);
}

const unsigned char&  InputBuffer::get_reply_data(std::size_t idx) const {

	this->check_ID(0x21);

	if (!(idx < 35)) {
		throw std::out_of_range("Index must be less than 35.");
	}
	return buf[15 + idx];
}

const ByteSubVector InputBuffer::get_MCU_FW_update_report() const {
	this->check_ID(0x23);
	return ByteSubVector(buf.begin() + 13, buf.begin() + 49 + 1);
}

const unsigned char&  InputBuffer::get_MCU_FW_update_report(std::size_t idx) const {

	this->check_ID(0x23);

	if (!(idx < 37)) {
		throw std::out_of_range("Index must be less than 37.");
	}
	return buf[13 + idx];
}

const Gyro InputBuffer::get_Gyro() {

}

const Accel InputBuffer::get_Acc() {

}

Gyro InputBuffer::get_Gyro() {

	this->check_ID({0x30, 0x31, 0x32, 0x33});

	return Gyro();
}

Accel InputBuffer::get_Acc() {

	this->check_ID({ 0x30, 0x31, 0x32, 0x33 });

	return Accel();
}

const ByteSubVector InputBuffer::get_NFC_IR_input_report() const {
	this->check_ID(0x21);
	return ByteSubVector(buf.begin() + 49, buf.begin() + 361 + 1);
}

const unsigned char&  InputBuffer::get_NFC_IR_input_report(std::size_t idx) const {

	this->check_ID(0x31);

	if (!this->enabledNFC()) {
		throw std::runtime_error("Wrong buffer size. NFC/IR require buffer of size 361.");
	}

	if (!(idx < 313)) {
		throw std::out_of_range("Index must be less than 35.");
	}

	return buf[49 + idx];
}

void InputBuffer::check_ID(unsigned char valid) const {
	const unsigned char& ID = this->get_cmd();
	if (ID != valid) {
		std::stringstream error;
		error << "Wrong mode! ID should be " << std::hex << static_cast<unsigned int>(valid) 
			<< ", but ID is " << std::hex << static_cast<unsigned int>(ID) << std::endl;
		throw std::runtime_error(error.str());
	}
}

void InputBuffer::check_ID(std::unordered_set<unsigned char> valid_list) const {
	const unsigned char& ID = this->get_cmd();
	if (valid_list.find(ID) == valid_list.end()) {
		std::stringstream error;
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

OutputBuffer::OutputBuffer(std::size_t dataSize) : BufferBase(11 + dataSize) {

	this->set_RL(0x00, 0x01, 0x40, 0x40);
	this->set_RR(0x00, 0x01, 0x40, 0x40);
}

void OutputBuffer::set_cmd(unsigned char in) {
	buf[0] = in;
}

void OutputBuffer::set_GP(unsigned char in) {
	buf[1] = in;
}

void OutputBuffer::set_subcmd(unsigned char in) {
	buf[10] = in;
}

void OutputBuffer::set_RL(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	buf[2] = a;
	buf[3] = b;
	buf[4] = c;
	buf[5] = d;
}

void OutputBuffer::set_RR(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	buf[6] = a;
	buf[7] = b;
	buf[8] = c;
	buf[9] = d;
}

void OutputBuffer::set_data(std::vector<unsigned char> data) {

	if (data.size() + 11 != buf.size()) {
		throw std::out_of_range("Data does not fit in the OutputBuffer.");
	}

	std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
}
