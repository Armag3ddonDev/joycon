#pragma once

#include <thread>

#include "hidapi.h"

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

	void capture();
	void callback();

	template<typename T>
	void printBuf(const std::vector<T>& buf, const unsigned int size);

private:
	hid_device * handle;
	std::thread callback_thread;
	bool alive = true;
};
