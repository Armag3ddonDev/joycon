#pragma once

#include <vector>

class Buffer {
public:
	Buffer(std::size_t size) : buf(size, 0) {}

	void printBuf(unsigned int size = 0) const;
	const std::size_t& size();

protected:
	std::vector<unsigned char> buf;
};

class OutputBuffer : public Buffer {
public:
	OutputBuffer() : Buffer(65) {}

	void clean();
	unsigned char& operator[](std::size_t idx);
	unsigned char* data();
};

class InputBuffer : public Buffer {
public:
	InputBuffer() : Buffer(65) {
		this->set_RL(0x00, 0x01, 0x40, 0x40);
		this->set_RR(0x00, 0x01, 0x40, 0x40);
	}

	void set_cmd(unsigned char in);		// command byte
	void set_GP(unsigned char in);		// global packet number (increments by 1 for each package sent;  It loops in 0x0 - 0xF range)	
	void set_subcmd(unsigned char in);	// subcommand byte

	// left rumble data
	void set_RL(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

	// right rumble data
	void set_RR(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

	// data
	void set_data(std::vector<unsigned char> data);

	const unsigned char* data();
};