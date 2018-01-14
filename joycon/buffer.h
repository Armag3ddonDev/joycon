#pragma once

#include <vector>
#include <string>
#include <unordered_set>

enum POWER{
	EMPTY,
	CRITICAL,
	LOW,
	MEDIUM,
	FULL
};

typedef std::vector<unsigned char>::iterator byte_iterator;
typedef std::vector<unsigned char>::const_iterator const_byte_iterator;

// function collection for byte-containers
class ByteBase {

public:

	virtual std::size_t size() const = 0;
	virtual const_byte_iterator begin() const = 0;
	virtual const_byte_iterator end() const = 0;

	unsigned long int to_int(std::size_t start,            std::size_t length, bool bigEndian = true) const;
	unsigned long int to_int(const_byte_iterator it_begin, std::size_t length, bool bigEndian = true) const;

	std::string to_hex_string(std::size_t start,            std::size_t length,         std::string prefix = "0x", std::string delimiter = "") const;
	std::string to_hex_string(const_byte_iterator it_begin, const_byte_iterator it_end, std::string prefix = "0x", std::string delimiter = "") const;
	
	void print(unsigned int size = 0) const;
	void print(const_byte_iterator it_begin, const_byte_iterator it_end) const;
};

// non-resizable minimal std::vector<unsigned char> class
class ByteVector : public ByteBase {
public:
	ByteVector() {}
	ByteVector(std::size_t n) : vec(n, 0) {}
	ByteVector(const_byte_iterator it_start, const_byte_iterator it_end) : vec(it_start, it_end) {}
	ByteVector(const ByteVector&) = default;

	std::size_t size() const { return vec.size(); }

	unsigned char* data() { return vec.data(); }
	const unsigned char* data() const { return vec.data(); }

	byte_iterator begin() { return vec.begin(); }
	byte_iterator end() { return vec.end(); }
	const_byte_iterator begin() const { return vec.begin(); }
	const_byte_iterator end() const { return vec.end(); }

	unsigned char& operator[](std::size_t idx) { return vec[idx]; }
	const unsigned char& operator[](std::size_t idx) const { return vec[idx]; }

	unsigned char& at(std::size_t idx) { return vec.at(idx); }
	const unsigned char& at(std::size_t idx) const { return vec.at(idx); }

private:
	std::vector<unsigned char> vec;
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
	InputBuffer(bool bEnabledNFC = false);

	bool enabledNFC() const { return buf.size() == 362; }
	void clean();
	inline unsigned char* data() { return buf.data(); }
	inline const unsigned char* data() const { return buf.data(); }

	const unsigned char& get_cmd() const;

	const unsigned char& get_timer() const;

	POWER get_battery_level() const;

	// get_button_status() const;

	// get_left_analog_stick_data() const;

	// get_right_analog_stick_data() const;

	// get_vibrator_input_report() const;

	// ID 21
	const unsigned char& get_ACK() const;

	// ID 21
	const unsigned char& get_subcommandID_reply() const;

	// ID 21
	ByteVector get_reply_data() const;
	const unsigned char&  get_reply_data(std::size_t idx) const;

	// ID 23
	ByteVector get_MCU_FW_update_report() const;

	// ID 30, 31, 32, 33
	ByteVector get_AxisData() const;

	// ID 31
	ByteVector get_NFC_IR_input_report() const;

private:
	void check_ID(unsigned char valid) const;
	void check_ID(std::unordered_set<unsigned char> valid_list) const;
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
	OutputBuffer(OutputBuffer&) = default;

	inline const unsigned char* data() const { return buf.data(); }

	/// set command byte
	void set_cmd(unsigned char in);	

	/// set global packet number (increments by 1 for each package sent;  It loops in 0x0 - 0xF range)
	void set_GP(unsigned char in);	

	/// set subcommand byte
	void set_subcmd(unsigned char in);	

	/// set left rumble data
	void set_RL(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

	/// set right rumble data
	void set_RR(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

	/// set data
	void set_data(const std::vector<unsigned char>& data);
};
