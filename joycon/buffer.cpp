#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

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
	return this->to_hex_string(this->begin(), this->end(), prefix, delimiter);
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

	if(size == 0) {
		size = this->size();
	} else if (size > this->size()) {
		throw std::out_of_range("Size is too big!");
	}
	std::cout << this->to_hex_string(this->begin(), this->begin() + size, "", " ");
}

template <typename T>
void ByteBase<T>::print(const_byte_iterator it_begin, const_byte_iterator it_end) const {
	std::cout << this->to_hex_string(it_begin, it_end, "", " ") << std::endl;
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

OutputBuffer::OutputBuffer(std::size_t dataSize) : BufferBase((11 + dataSize < 11)? throw std::bad_alloc() : 11+dataSize) {

	this->set_rumble(JOYCON::LEFT , 0x00, 0x01, 0x40, 0x40);
	this->set_rumble(JOYCON::RIGHT, 0x00, 0x01, 0x40, 0x40);
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

void OutputBuffer::set_rumble(JOYCON type, unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	
	bool offset = false;
	if (type == JOYCON::LEFT) {
		offset = false;
	} else if (type == JOYCON::RIGHT) {
		offset = true;
	} else {
		throw std::invalid_argument("Invalid JOYCON type.");
	}

	buf[2 + offset*4] = a;	// L(2), R(6)
	buf[3 + offset*4] = b;	// L(3), R(7)
	buf[4 + offset*4] = c;	// L(4), R(8)
	buf[5 + offset*4] = d;	// L(5), R(9)
}

void OutputBuffer::set_rumble(JOYCON type, double frequency, double amplitude) {

	bool offset = false;
	if (type == JOYCON::LEFT) {
		offset = false;
	} else if (type == JOYCON::RIGHT) {
		offset = true;
	} else {
		throw std::invalid_argument("Invalid JOYCON type.");
	}

	uint16_t hf;
	uint8_t lf;
	uint8_t hf_amp;
	uint16_t lf_amp;

	encode_frequency(frequency, hf, lf);
	encode_amplitude(amplitude, hf_amp, lf_amp);

	//Byte swapping
	buf[2 + offset * 4] = hf & 0xFF;
	buf[3 + offset * 4] = hf_amp + ((hf >> 8) & 0xFF); //Add amp + 1st byte of frequency to amplitude byte
	//Byte swapping
	buf[4 + offset * 4] = lf + ((lf_amp >> 8) & 0xFF); //Add freq + 1st byte of LF amplitude to the frequency byte
	buf[5 + offset * 4] = lf_amp & 0xFF;
}

void OutputBuffer::set_data(const ByteVector& data) {

	if (data.size() + 11 != buf.size()) {
		throw std::length_error("Size mismatch. Data does not fit in the OutputBuffer.");
	}

	std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
}


void OutputBuffer::encode_frequency(double frequency, uint16_t& hf, uint8_t& lf) const {

	// Float frequency to hex conversion
	if (frequency < 40.87) {
		frequency = 40.87;
	}
	else if (frequency > 1252.57) {
		frequency = 1252.57;
	}

	// maps to 0x41(65) - 0xDF(223)
	uint8_t encoded_hex_freq = static_cast<uint8_t>(round(log2(frequency / 10.0)*32.0));

	// Convert to Joy-Con HF range. Range in big-endian: 0x0004(81.75Hz)-0x01FC(1252.57Hz) with +0x0004 steps.
	hf = (encoded_hex_freq - 0x60) * 4;	// what happens under 81.75Hz?
										// Convert to Joy-Con LF range. Range: 0x01-0x7F.
	lf = encoded_hex_freq - 0x40;
}

void OutputBuffer::encode_amplitude(double amplitude, uint8_t& hf_amp, uint16_t& lf_amp) const {

	// clamp amplitude
	if ((amplitude < 0) || (amplitude > 1.002867)) {	// dont increase for safety-reasons of the linear resonant actuators.
		throw std::invalid_argument("amplitude must be between 0 and 1.002867");
	}

	// maps to 0x00(0) - 0xC8(100)
	uint8_t amp_encoded;
	if (amplitude < 0.008) {
		amp_encoded = 0x00;
	} else if (amplitude < 0.112491) {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 9.0)*4.0)) - 1;
	}
	else if (amplitude < 0.224982) {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 6.0)*16.0)) - 1;
	}
	else {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 5.0)*32.0)) - 1;
	}

	// saveguard
	if (amp_encoded > 0xC8) { 
		throw std::bad_exception(); 
	}

	hf_amp = amp_encoded << 1;	// multiply by 2
	lf_amp = 0x8000 * (amp_encoded & 0x01) + 0x40 + (amp_encoded >> 1);	// 0x80XX if odd, else 0x00XX
}