#include <iostream>
#include <sstream>
#include <algorithm>

#include "buffer.h"

/* ------ BYTE BASE ----- */

template <typename T>
unsigned long int ByteBase<T>::to_int(bool bigEndian) const { 
	return to_int(this->begin(), this->size(), bigEndian);
}

template <typename T>
unsigned long int ByteBase<T>::to_int(std::size_t start, std::size_t length, bool bigEndian) const {

	if (length + start > this->size()) {
		throw std::out_of_range("Length is too big!");
	}

	return this->to_int(this->begin() + start, length, bigEndian);
}

template <typename T>
unsigned long int ByteBase<T>::to_int(const_byte_iterator it_begin, std::size_t length, bool bigEndian) const {

	if (sizeof(unsigned long int) < length) {
		throw std::overflow_error("Can not convert to unsigned long int - length is too big.");
	}

	unsigned long int res{ 0 };

	for (std::size_t i = 0; i < length; ++i) {
		std::size_t idx = (bigEndian) ? i : length - 1 - i;
		res = (res << 8) + *(it_begin + idx);
	}

	return res;
}

template <typename T>
std::string ByteBase<T>::to_hex_string(std::string prefix, std::string delimiter) const {
	return to_hex_string(this->begin(), this->end(), prefix, delimiter);
}

template <typename T>
std::string ByteBase<T>::to_hex_string(std::size_t start, std::size_t length, std::string prefix, std::string delimiter) const {

	if (length + start > this->size()) {
		throw std::out_of_range("Length is too big!");
	}

	return this->to_hex_string(this->begin() + start, this->begin() + start + length, prefix, delimiter);
}

template <typename T>
std::string ByteBase<T>::to_hex_string(const_byte_iterator it_begin, const_byte_iterator it_end, std::string prefix, std::string delimiter) const {

	std::ostringstream sstream;
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

template <typename T>
void ByteBase<T>::print(unsigned int size) const {
	if (size > this->size()) {
		throw std::out_of_range("Size is too big!");
	}
	std::cout << this->to_hex_string(this->begin(), this->begin() + size, "", " ");
}

template <typename T>
void ByteBase<T>::print(const_byte_iterator it_begin, const_byte_iterator it_end) const {

	std::cout << to_hex_string(it_begin, it_end, "", " ") << std::endl;
}

template <typename T>
void ByteBase<T>::swap() {
	byte_iterator first = this->begin();
	byte_iterator last = this->end();
	while ((first != last) && (first != --last)) {
		std::iter_swap(first, last);
		++first;
	}
}

template <typename T>
T ByteBase<T>::swapped() const {
	T res(this->begin(), this->end());
	std::size_t n = this->size();
	for (std::size_t i = 0; i < n; ++i) {
		res[i] = *(this->begin() + i);
	}
	return res;
}

template class ByteBase<ByteVector>;

/* ---- INPUT BUFFER ---- */

void InputBuffer::clean() {
	std::fill(buf.begin(), buf.end(), 0);
}

const unsigned char& InputBuffer::get_ID() const {
	return buf[0];
}

const unsigned char& InputBuffer::get_timer() const {
	return buf[1];
}

POWER InputBuffer::get_battery_level() const {

	unsigned char battery_level = buf[2] >> 4;
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

const unsigned char& InputBuffer::get_ACK() const {
	this->check_ID(0x21);
	return buf[13];
}

const unsigned char& InputBuffer::get_subcommandID_reply() const {
	this->check_ID(0x21);
	return buf[14];
}

ByteVector InputBuffer::get_reply_data(std::size_t offset, std::size_t length) const {

	this->check_ID(0x21);

	if (length == 0) {
		length = 35;
	}

	if (offset + length > 35) {
		throw std::out_of_range("Length is too big!");
	}

	return ByteVector(buf.begin() + 15 + offset, buf.begin() + 15 + offset + length);
}

const unsigned char&  InputBuffer::get_reply_data_at(std::size_t idx) const {

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

void InputBuffer::check_ID(unsigned char valid) const {
	const unsigned char& ID = this->get_ID();
	if (ID != valid) {
		std::ostringstream error;
		error << "Wrong mode! ID should be " << std::hex << static_cast<unsigned int>(valid) 
			<< ", but ID is " << std::hex << static_cast<unsigned int>(ID) << std::endl;
		throw std::runtime_error(error.str());
	}
}

void InputBuffer::check_ID(std::unordered_set<unsigned char> valid_list) const {
	const unsigned char& ID = this->get_ID();
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

	/*
	// Float frequency to hex conversion
	if (freq < 0.0) {
		freq = 320.0;
	} else if (freq > 1252.0) {
		freq = 1252.0;
	}
	// encoded_hex_freq: -inf to 0xDF
	uint8_t encoded_hex_freq = (uint8_t)round(log2((double)freq / 10.0)*32.0);

	// Convert to Joy-Con HF range. Range in big-endian: 0x0004-0x01FC with +0x0004 steps.
	uint16_t hf = (encoded_hex_freq - 0x60) * 4;
	// Convert to Joy-Con LF range. Range: 0x01-0x7F.
	uint8_t lf = encoded_hex_freq - 0x40;
	*/

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

void OutputBuffer::set_data(const ByteVector& data) {

	if (data.size() + 11 != buf.size()) {
		throw std::length_error("Size msimatch. Data does not fit in the OutputBuffer.");
	}

	std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
}