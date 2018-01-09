#include <iostream>
#include <cmath>
#include <thread>
#include <vector>

#include "hidapi.h"
#include "joycon.h"

#define MAX_STR 255

#define JOYCON_VENDOR 0x057e

int main() {

	std::vector<Joycon> joycons;

	// Initialize the hidapi library
	if (hid_init()) {
		std::cerr << "HID initialization failed!" << std::endl;
		return -1;
	}

	std::cout << "Searching for devices..." << std::endl;

	hid_device_info* devs = hid_enumerate(JOYCON_VENDOR, 0x0);
	if (devs == NULL) {
		std::cout << "No bluetooth device detected!" << std::endl;
	}

	std::cout << "-----------------------------" << std::endl;
	for (hid_device_info* current = devs; current != NULL; current = current->next) {

		if (current->vendor_id != JOYCON_VENDOR) {
			continue;
		}

		switch (current->product_id) {
		case JOYCON_L_BT:
			break;
		case JOYCON_R_BT:
			break;
		case PRO_CONTROLLER:
			std::cout << "Pro-controller is not supported" << std::endl;
			continue;
		case JOYCON_CHARGING_GRIP:
			std::cout << "Joy-con charging grip is not supported" << std::endl;
			continue;
		default:
			continue;
		}

		joycons.emplace_back(static_cast<JOY_TYPE>(current->product_id), current->serial_number);

		std::cout << "-----------------------------" << std::endl;
	}

	hid_free_enumeration(devs);

	if (joycons.size() == 0) {
		std::cout << "No joy-con device detected!" << std::endl;
		return 0;
	}
	else {
		std::cout << "Starting capture for " << joycons.size() << " devices!" << std::endl;
		for (Joycon& jc : joycons) {
			jc.capture();
		}
	}

	std::cin.get();

	// Finalize the hidapi library
	hid_exit();
}
