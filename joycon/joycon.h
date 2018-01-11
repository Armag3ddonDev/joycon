#pragma once

#include "buffer.h"
#include "hidapi/hidapi.h"

#define MAX_STR 255

#define JOYCON_VENDOR 0x057e

enum JOY_TYPE {
	JOYCON_L_BT = 0x2006,
	JOYCON_R_BT = 0x2007,
	PRO_CONTROLLER = 0x2009,
	JOYCON_CHARGING_GRIP = 0x200e
};

class Joycon {
public:

	Joycon(Joycon&) = delete;
	Joycon(Joycon&&) = default;
	Joycon(JOY_TYPE type, wchar_t* serial_number);

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

	void send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data);

	void capture();
	void callback();
	void processReply(OutputBuffer& buf_out);

private:
	hid_device * handle;
	std::thread callback_thread;
	bool alive = true;
	std::size_t package_number = 0;
};
