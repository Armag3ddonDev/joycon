#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

#ifdef _WIN32
#include "hidapi.h"
#elif __linux__
#include <hidapi/hidapi.h>
#endif

#include "joycon.h"

static sig_atomic_t volatile shutdown_flag = 0;
static void SigCallback(int sig)
{
	shutdown_flag = 1;
}

int main() {

	// Initialize the hidapi library
	if (hid_init()) {
		std::cerr << "HID initialization failed!" << std::endl;
		return -1;
	}

	signal(SIGINT , SigCallback);
	signal(SIGTERM, SigCallback);

	JoyconVec joycons;
	if (joycons.addDevices()   == -1) { hid_exit();  return 0; }
	if (joycons.startDevices() == -1) { hid_exit();  return 0; };

	while (!shutdown_flag) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	hid_exit();
	return 0;
}
