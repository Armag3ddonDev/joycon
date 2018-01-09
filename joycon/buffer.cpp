#include <iostream>

#include "buffer.h"

//Buffer

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

const std::size_t& Buffer::size() { 
	return buf.size(); 
}

//OutputBuffer

void OutputBuffer::clean() {
	std::fill(buf.begin(), buf.end(), 0);
}

unsigned char& OutputBuffer::operator[](std::size_t idx) { 
	return buf[idx]; 
}

unsigned char* OutputBuffer::data() { 
	return buf.data(); 
}

//InputBuffer

void InputBuffer::set_cmd(unsigned char in) { // command byte
	buf[0] = in; 
}

void InputBuffer::set_GP(unsigned char in) { // global packet number (increments by 1 for each package sent;  It loops in 0x0 - 0xF range)
	buf[1] = in; 
}

void InputBuffer::set_subcmd(unsigned char in) { // subcommand byte
	buf[10] = in; 
}

// left rumble data
void InputBuffer::set_RL(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	buf[2] = a;
	buf[3] = b;
	buf[4] = c;
	buf[5] = d;
}

// right rumble data
void InputBuffer::set_RR(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	buf[6] = a;
	buf[7] = b;
	buf[8] = c;
	buf[9] = d;
}

// data
void InputBuffer::set_data(std::vector<unsigned char> data) {
	if (data.size() + 11 > buf.size()) {
		std::cout << "WARNING: skipping data of size " << data.size() << ". Exceding max buffer size." << std::endl;
	}
	else {
		std::copy(std::begin(data), std::end(data), std::begin(buf) + 11);
	}
}

const unsigned char* InputBuffer::data() {
	return buf.data();
}