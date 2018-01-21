#pragma once

#include <unordered_set>

#include "rumble.h"
#include "types.h"

// only expose some functions of ByteVector to BufferBase
class BufferBase {
public:
	BufferBase(std::size_t size) : buf(size, 0) {}
	BufferBase(const BufferBase& other) : buf(other.buf) {}

	std::size_t size() const { return buf.size(); }
	explicit operator ByteVector() { return buf; }

protected:
	ByteVector buf;
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

	friend std::ostream& operator<<(std::ostream& os, const InputBuffer& in);

private:
	void check_ID(byte valid) const;
	void check_ID(std::unordered_set<byte> valid_list) const;
};

inline std::ostream& operator<<(std::ostream& os, const InputBuffer& in) {
	os << to_hex_string(in.buf, 0, 1) << " | " << to_hex_string(in.buf, 1, 1) << " | " << to_hex_string(in.buf, 2, 1) << " | ";
	os << to_hex_string(in.buf, 3, 3) << " | " << to_hex_string(in.buf, 6, 3) << " | " << to_hex_string(in.buf, 9, 3) << " | ";
	os << to_hex_string(in.buf, 12, 1) << " |";

	if (in.get_ID() == 0x21) {
		os << " " << to_hex_string(in.buf, 13, 1) << " | " << to_hex_string(in.buf, 14, 1) << " | " << to_hex_string(in.buf, 15, 35) << " |";
	} else if (in.get_ID() == 0x23) {
		os << " " << to_hex_string(in.buf, 13, 37) << " |";
	} else {
		os << "| " << to_hex_string(in.buf, 13, 12) << " | " << to_hex_string(in.buf, 25, 12) << " | " << to_hex_string(in.buf, 37, 12) << " ||";
		if (in.get_ID() == 0x31 && in.enabledNFC()) {
			os << " " << to_hex_string(in.buf, 49, 313) << " | ";
		}
	}

	return os;
}

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

	friend std::ostream& operator<<(std::ostream& os, const OutputBuffer& in);

};

inline std::ostream& operator<<(std::ostream& os, const OutputBuffer& in) {
	os << to_hex_string(in.buf, 0, 1) << " | " << to_hex_string(in.buf, 1, 1) << " | " ;
	os << to_hex_string(in.buf, 2, 4) << " | " << to_hex_string(in.buf, 6, 4) << " | ";
	os << to_hex_string(in.buf, 10, 1) << " |";
	if (in.buf.size() > 11) {
		os << " " << to_hex_string(in.buf.begin() + 11, in.buf.end()) << " |";
	}
	return os;
}