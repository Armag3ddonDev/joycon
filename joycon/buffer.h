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

struct Gyro {
	double x = 0;
	double y = 0;
	double z = 0;
};

struct Accel {
	double x = 0;
	double y = 0;
	double z = 0;
};

// non-resizable minimal std::vector<unsigned char> class
class ByteVector {
public:
	ByteVector() {}
	ByteVector(std::size_t n) : vec(n, 0) {}
	ByteVector(ByteVector&) = default;

	unsigned long int to_int(std::size_t pos, std::size_t length, bool bigEndian = true);
	std::string to_hex_string(std::size_t start, std::size_t length, std::string prefix = "0x", std::string delimiter = "") const;
	void print(unsigned int size = 0) const;
	const std::size_t size() const { return vec.size(); }

	unsigned char* data() { return vec.data(); }
	const unsigned char* data() const { return vec.data(); }

	typedef std::vector<unsigned char>::iterator iterator;
	typedef std::vector<unsigned char>::const_iterator const_iterator;

	iterator begin() { return vec.begin(); }
	iterator end() { return vec.end(); }
	const_iterator begin() const { return vec.begin(); }
	const_iterator end() const { return vec.end(); }

	unsigned char& operator[](std::size_t idx) { return vec[idx]; }
	const unsigned char& operator[](std::size_t idx) const { return vec[idx]; }

	unsigned char& at(std::size_t idx) { return vec.at(idx); }
	const unsigned char& at(std::size_t idx) const { return vec.at(idx); }

private:
	std::vector<unsigned char> vec;
};

// only expose some functions of ByteVector
class BufferBase : protected ByteVector {
public:
	BufferBase(std::size_t size) : ByteVector(size), buf(*this) {}
	BufferBase(BufferBase& other) : ByteVector(other), buf(*this) {}

	using ByteVector::to_hex_string;
	using ByteVector::print;
	using ByteVector::size;

protected:
	ByteVector& buf;
};


// ID 21		: size 50
// ID 23		: size 50
// ID 30/32/33	: size 49
// ID 31		: size 362
class InputBuffer : public BufferBase {
public:
	InputBuffer(bool bEnabledNFC = false);
	InputBuffer(InputBuffer&) = default;

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

	const unsigned char& get_ACK() const;

	const unsigned char& get_subcommandID_reply() const;

	const unsigned char&  get_reply_data(std::size_t idx) const;
	ByteVector::const_iterator get_reply_data_start() const;
	ByteVector::const_iterator get_reply_data_end() const;

	const unsigned char&  get_MCU_FW_update_report(std::size_t idx) const;
	ByteVector::const_iterator get_MCU_FW_update_report_start() const;
	ByteVector::const_iterator get_MCU_FW_update_report_end() const;

	Gyro get_Gyro();

	Accel get_Acc();

	const unsigned char&  get_NFC_IR_input_report(std::size_t idx) const;
	ByteVector::const_iterator get_NFC_IR_input_report_start() const;
	ByteVector::const_iterator get_NFC_IR_input_report_end() const;

private:
	void check_ID(unsigned char valid) const;
	void check_ID(std::unordered_set<unsigned char> valid_list) const;
};

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
	void set_data(std::vector<unsigned char> data);
};
