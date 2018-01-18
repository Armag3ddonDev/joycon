#pragma once

#include <vector>
#include <array>
#include <sstream>
#include <string>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using byte = unsigned char;
using ByteVector = std::vector<byte>;

template <std::size_t N>
using ByteArray = std::array<byte, N>;

/* TYPE TRAITS */

template <typename T>
struct is_valid_container {
	enum { value = false };
};

template <>
struct is_valid_container<ByteVector> {
	enum { value = true };
};

template <std::size_t N>
struct is_valid_container<ByteArray<N>> {
	enum { value = true };
};

// usage:
// template <typename T>
// typename std::enable_if<is_valid_container<T>::value, RETURN_TYPE>::type
// f(T const&)

// check for byte-type containers:
// template <typename T>
// typename std::enable_if<std::is_same<typename T::value_type, byte>::value, RETURN_TYPE>::type
// f(T const&)

/* HELPER FUNCTIONS */

template <typename const_iterator>
typename std::enable_if<std::is_same<typename const_iterator::value_type, byte>::value, unsigned long int>::type
to_int(const_iterator it_begin, const_iterator it_end, bool bigEndian = true) {

	unsigned long int res{ 0 };

	int length = std::distance(it_begin, it_end);

	if (length < 0) {
		throw std::underflow_error("it_begin > it_end");
	}

	if (sizeof(unsigned long int) < length) {
		throw std::overflow_error("Can not convert to unsigned long int - length is too big.");
	}

	if (bigEndian) {
		for (auto it = it_begin; it != it_end; ++it) {
			res = (res << 8) + *it;
		}
	} else {
		for (auto it = it_end - 1; it != it_begin - 1; --it) {
			res = (res << 8) + *it;
		}
	}

	return res;
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value, unsigned long int>::type
to_int(const T& container, bool bigEndian = true) {
	return to_int(container.begin(), container.end(), bigEndian);
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value, unsigned long int>::type
to_int(const T& container, std::size_t start, std::size_t length, bool bigEndian = true) {
	return to_int(container.begin() + start, container.begin() + start + length, bigEndian);
}

template <typename const_iterator>
typename std::enable_if<std::is_same<typename const_iterator::value_type, byte>::value, std::string>::type
to_hex_string(const_iterator it_begin, const_iterator it_end, std::string prefix = "0x", std::string delimiter = "") {

	int length = std::distance(it_begin, it_end);
	if (length < 0) {
		throw std::underflow_error("it_begin > it_end");
	}

	std::ostringstream sstream;
	if (it_begin != it_end) {
		sstream << prefix;
	}

	std::string del = "";

	for (const_iterator it = it_begin; it != it_end; ++it) {

		const byte& current_byte = *it;

		std::string fill = "";
		if (current_byte <= 0x0F) { fill = "0"; }

		sstream << del << fill << std::hex << static_cast<unsigned int>(current_byte);

		del = delimiter;
	}

	return sstream.str();
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value, std::string>::type
to_hex_string(const T& container, std::string prefix = "0x", std::string delimiter = "") {
	return to_hex_string(container.begin(), container.end(), prefix, delimiter);
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value, std::string>::type
to_hex_string(const T& container, std::size_t start, std::size_t length, std::string prefix = "0x", std::string delimiter = "") {
	return to_hex_string(container.begin() + start, container.begin() + start + length, prefix, delimiter);
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value, std::ostream& >::type
operator<<(std::ostream& os, const T& container) {
	os << to_hex_string(container.begin(), container.end(), "", " ");
	return os;
}

template <typename T>
typename std::enable_if<std::is_same<typename T::value_type, byte>::value>::type
print(const T& container, std::size_t size = 0, std::string prefix = "", std::string delimiter = "") {

	if (size == 0) {
		size = container.size();
	} else if (size > container.size()) {
		throw std::out_of_range("Size is larger than container.");
	}

	std::cout << to_hex_string(container.begin(), container.begin() + size, prefix, delimiter) << std::endl;
}

/* HELPER TYPES */

enum JOY_PID {
	JOYCON_L_BT = 0x2006,
	JOYCON_R_BT = 0x2007,
	PRO_CONTROLLER = 0x2009,
	JOYCON_CHARGING_GRIP = 0x200e
};

enum POWER {
	EMPTY,
	CRITICAL,
	LOW,
	MEDIUM,
	FULL
};

struct JoyconDeviceInfo {
	std::string firmwareVersion;	// Firmware Version. Latest is 3.86 (from 4.0.0 and up).
	unsigned int joyconType;		// 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller
	std::string mac;				// Joy-Con MAC address in Big Endian
	bool useColorsSPI;				// If true, colors in SPI are used. Otherwise default ones.
};

struct TriggerButtonElapsedTime {
	std::chrono::milliseconds L, R, ZL, ZR, SL, SR, HOME;
};

// 'on' overrides 'flashing'. When on USB, flashing bits work like always on bits.
enum PLAYER_LIGHTS {
	P0_KEEP_ON = 1 << 0,
	P1_KEEP_ON = 1 << 1,
	P2_KEEP_ON = 1 << 2,
	P3_KEEP_ON = 1 << 3,
	P0_FLASH = 1 << 4,
	P1_FLASH = 1 << 5,
	P2_FLASH = 1 << 6,
	P3_FLASH = 1 << 7
};

constexpr PLAYER_LIGHTS operator|(PLAYER_LIGHTS X, PLAYER_LIGHTS Y) {
	return static_cast<PLAYER_LIGHTS>(static_cast<unsigned char>(X) | static_cast<unsigned char>(Y));
}

constexpr PLAYER_LIGHTS operator&(PLAYER_LIGHTS X, PLAYER_LIGHTS Y) {
	return static_cast<PLAYER_LIGHTS>(static_cast<unsigned char>(X) & static_cast<unsigned char>(Y));
}