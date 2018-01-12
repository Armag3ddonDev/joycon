#include <iostream>

#include "buffer.h"

/* ------- BUFFER ------- */

void Buffer::printBuf(unsigned int size) const {

	if (size == 0 || size > buf.size()) {
		size = buf.size();
	}

	for (unsigned int i = 0; i < size; ++i) {
		const unsigned int& hx = buf[i];
		if (hx <= 0xf)
			std::cout << std::hex << 0 << hx << " "; //leading 0
		else
			std::cout << std::hex << hx << " ";
	}
	std::cout << std::endl;
}

const std::size_t Buffer::size() {
	return buf.size();
}

/* ---- INPUT BUFFER ---- */

void InputBuffer::clean() {
	std::fill(buf.begin(), buf.end(), 0);
}

unsigned char& InputBuffer::operator[](std::size_t idx) { 
	return buf[idx];
}

unsigned char* InputBuffer::data() { 
	return buf.data();
}

unsigned char& InputBuffer::cmd() {
	return buf[0];
}

/* ---- OUTPUT BUFFER --- */

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

	if (data.size() + 11 > buf.size()) {
		std::cout << "WARNING: skipping data of size " << data.size() << ". Exceding max buffer size." << std::endl;
	} else {
		std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
	}
}

const unsigned char* OutputBuffer::data() {
	return buf.data();
}
