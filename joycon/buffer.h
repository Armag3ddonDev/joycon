#pragma once

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>
#include <bitset>

#include "rumble.h"

enum POWER{
	EMPTY,
	CRITICAL,
	LOW,
	MEDIUM,
	FULL
};

using byte = unsigned char;

// restrict containers and add new functionality
template <typename T>
class ByteContainer {

public:
	template <typename... Args>
	ByteContainer(Args && ...args) : T(std::forward<Args>(args)...) {}
	ByteContainer(const T& other) : vec(other.vec) {}

	std::size_t size() const { return vec.size(); }

	byte* data() { return vec.data(); }
	const byte* data() const { return vec.data(); }

	using iterator = typename T::iterator;
	using const_iterator = typename T::iterator;

	iterator begin() { return vec.begin(); }
	iterator end() { return vec.end(); }
	const_iterator begin() const { return vec.begin(); }
	const_iterator end() const { return vec.end(); }

	byte& operator[](std::size_t idx) { return vec[idx]; }
	const byte& operator[](std::size_t idx) const { return vec[idx]; }

	byte& at(std::size_t idx) { return vec.at(idx); }
	const byte& at(std::size_t idx) const { return vec.at(idx); }

	unsigned long int to_int(bool bigEndian = true) const;
	unsigned long int to_int(std::size_t start, std::size_t length, bool bigEndian = true) const;
	unsigned long int to_int(const_iterator it_begin, std::size_t length, bool bigEndian = true) const;

	std::string to_hex_string(std::string prefix = "0x", std::string delimiter = "") const;
	std::string to_hex_string(std::size_t start, std::size_t length, std::string prefix = "0x", std::string delimiter = "") const;
	std::string to_hex_string(const_iterator it_begin, const_iterator it_end, std::string prefix = "0x", std::string delimiter = "") const;

	void print(unsigned int size = 0) const;
	void print(const_iterator it_begin, const_iterator it_end) const;

	operator long unsigned int() { return this->to_int(this->begin(), this->size()); }

private:

	T vec;
};

class ByteVector : public ByteContainer<std::vector<byte>> {

public:
	ByteVector() {}
	ByteVector(std::size_t n) : ByteContainer<std::vector<byte>>(n, 0) {}
	ByteVector(std::initializer_list<byte> list) : ByteContainer<std::vector<byte>>(list) {}
	ByteVector(const_iterator it_start, const_iterator it_end) : ByteContainer<std::vector<byte>>(it_start, it_end) {}
	ByteVector(const ByteVector&) = default;
};

template <std::size_t N>
class ByteArray : public ByteContainer<std::array<byte, N>> {
public:
	ByteArray() {}
	ByteArray(std::initializer_list<byte> list) : ByteContainer<std::array<byte, N>>(list) {}
	ByteArray(const ByteArray<N>&) = default;
};

// only expose some functions of ByteVector to BufferBase
class BufferBase : protected ByteVector {
public:
	BufferBase(std::size_t size) : ByteVector(size), buf(*this) {}
	BufferBase(const BufferBase& other) : ByteVector(other), buf(*this) {}

	using ByteVector::to_hex_string;
	using ByteVector::print;
	using ByteVector::size;

protected:
	ByteVector& buf;
};

// ID 21:	... | 13 | 14 | 15 - 49 (SUBCMD_reply)		size 50
// ID 23 :	... | 13 - 49 (MCU report)					size 50
// ID 30 :	... | 13 - 48 (Axis/Gyro/Accel)				size 49
// ID 32 :	... | 13 - 48 (Axis/Gyro/Accel)				size 49
// ID 33 :	... | 13 - 48 (Axis/Gyro/Accel)				size 49
// ID 31 :	... | 13 - 48 (Axis/Gyro/Accel) | 49 - 361	size 362
// --> InputBuffer size is eather 50 or 362, depending on if NFC is enabled
class InputBuffer : public BufferBase {
public:
	InputBuffer(bool bEnabledNFC = false) : BufferBase(bEnabledNFC ? 362 : 50) {}

	bool enabledNFC() const { return buf.size() == 362; }
	void clean();
	inline byte* data() { return buf.data(); }
	inline const byte* data() const { return buf.data(); }

	const byte& get_ID() const;

	const byte& get_timer() const;

	POWER get_battery_level() const;

	// get_button_status() const;

	// get_left_analog_stick_data() const;

	// get_right_analog_stick_data() const;

	// get_vibrator_input_report() const;

	// ID 21
	const byte& get_ACK() const;

	// ID 21
	const byte& get_subcommandID_reply() const;

	// ID 21
	ByteVector get_reply_data(std::size_t offset = 0, std::size_t length = 0) const;
	const byte&  get_reply_data_at(std::size_t idx) const;

	// ID 23
	ByteVector get_MCU_FW_update_report() const;

	// ID 30, 31, 32, 33
	ByteVector get_AxisData() const;

	// ID 31
	ByteVector get_NFC_IR_input_report() const;

private:
	void check_ID(byte valid) const;
	void check_ID(std::unordered_set<byte> valid_list) const;
};


// byte 0		: CMD
// byte 1		: GP
// byte 2 - 5	: Rumble left	(default (no rumble): 00 01 40 40)
// byte 6 - 9	: Rumble right	(default (no rumble): 00 01 40 40)
// byte 10		: SUBCMD
// byte 11 - .. : data
class OutputBuffer : public BufferBase {
public:
	OutputBuffer(std::size_t dataSize = 0);
	OutputBuffer(OutputBuffer&) = delete;

	inline const byte* data() const { return buf.data(); }

	/// set command byte
	void set_cmd(byte in);	

	/// set global packet number (increments by 1 for each package sent;  It loops in 0x0 - 0xF range)
	void set_GP(byte in);	

	/// set subcommand byte
	void set_subcmd(byte in);	

	/// set left rumble data
	void set_rumble_left(const Rumble& rumble);

	/// set right rumble data
	void set_rumble_right(const Rumble& rumble);

	/// set data
	void set_data(const ByteVector& data);

};