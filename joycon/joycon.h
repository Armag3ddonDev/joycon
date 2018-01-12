#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <thread>

#ifdef _WIN32
#include "hidapi.h"
#elif __linux__
#include <hidapi/hidapi.h>
#endif

#include "buffer.h"

#define THROW(x) throw(std::runtime_error(std::string(__FILE__) + " - line " + std::to_string(__LINE__) + ": " + __FUNCTION__ + "(): " + x ))
#define CHECK(x) if (x == -1) {THROW(#x + " failed!");}

#define MAX_STR 255
#define JOYCON_VENDOR 0x057e

enum JOY_PID {
	JOYCON_L_BT = 0x2006,
	JOYCON_R_BT = 0x2007,
	PRO_CONTROLLER = 0x2009,
	JOYCON_CHARGING_GRIP = 0x200e
};

class Joycon {

public:
	Joycon(Joycon&) = delete;
	Joycon(Joycon&&) = default;
	Joycon(JOY_PID PID, wchar_t* serial_number);

	~Joycon();

	struct Gyro {
		double x = 0;
		double y = 0;
		double z = 0;
	} gyro;

	struct Accel {
		double x = 0;
		double y = 0;
		double z = 0;
	} accel;

	void printDeviceInfo() const;
	void send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data);
	void capture();
	void callback();
	void processReply(InputBuffer& buf_in);

private:
	hid_device* handle;
	std::thread callback_thread;
	bool alive = true;
	std::size_t package_number = 0;
};

class JoyconVec : public std::vector<Joycon> {
public:
	int addDevices();
	int startDevices();
};
