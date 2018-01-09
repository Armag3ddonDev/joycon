#include "stdafx.h"

#define MAX_STR 255

#define JOYCON_VENDOR 0x057e

enum JOY_TYPE {
	JOYCON_L_BT = 0x2006,
	JOYCON_R_BT = 0x2007,
	PRO_CONTROLLER = 0x2009,
	JOYCON_CHARGING_GRIP = 0x200e
};

class joycon {
public:

	joycon(joycon&) = delete;
	joycon(joycon&&) = default;

	joycon(JOY_TYPE type, wchar_t* serial_number): alive(true) {
		// Open the device using the VID, PID,
		// and optionally the Serial number.
		handle = hid_open(JOYCON_VENDOR, type, serial_number);

		std::vector<unsigned char> buf(65, 0);

		wchar_t wstr[MAX_STR];
		int res;

		// Read the Manufacturer String
		res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
		wprintf(L"Manufacturer String: %s\n", wstr);

		// Read the Product String
		res = hid_get_product_string(handle, wstr, MAX_STR);
		wprintf(L"Product String: %s\n", wstr);

		// Read the Serial Number String
		res = hid_get_serial_number_string(handle, wstr, MAX_STR);
		wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);

		// Read Indexed String 1
		res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
		wprintf(L"Indexed String 1: %s\n", wstr);

		std::cout << " >> Press Enter to continue << ";
		std::cin.get();
		
		unsigned int timing_byte = 0;

		hid_set_nonblocking(handle, 0);

		std::cout << "Enabling vibration..." << std::endl;
		uint8_t vib_buf[12] = { 0x01, (++timing_byte) & 0xF, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40, 0x48, 0x01 };
		memcpy(buf.data(), vib_buf, 12);
		res = hid_write(handle, buf.data(), 65);
		std::cout << "sent:     ";
		printBuf(buf, 32);

		res = hid_read(handle, buf.data(), 65);
		std::cout << "received: ";
		printBuf(buf, 32);

		std::fill(buf.begin(), buf.end(), 0); //cleanup
		std::cout << "Enabling IMU..." << std::endl;
		uint8_t gyro_buf[12] = { 0x01, (++timing_byte) & 0xF, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40, 0x40, 0x01 };
		memcpy(buf.data(), gyro_buf, 12);
		res = hid_write(handle, buf.data(), 65);
		std::cout << "sent:     ";
		printBuf(buf, 32);
		res = hid_read(handle, buf.data(), 65);
		std::cout << "received: ";
		printBuf(buf, 32);

		std::fill(buf.begin(), buf.end(), 0); //cleanup
		std::cout << "Increasing data rate for Bluetooth..." << std::endl;
		uint8_t rate_buf[12] = { 0x01, (++timing_byte) & 0xF, 0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40, 0x03, 0x30 };
		memcpy(buf.data(), rate_buf, 12);
		res = hid_write(handle, buf.data(), 65);
		std::cout << "sent:     ";
		printBuf(buf, 32);

		res = hid_read(handle, buf.data(), 65);
		std::cout << "received: ";
		printBuf(buf, 32);

		std::cout << " >> Press Enter to continue << ";
		std::cin.get();
	}

	template<typename T>
	void printBuf(const std::vector<T>& buf, const unsigned int size) {
		for (unsigned int i = 0; i < size; i++) {
			const unsigned int &hx = buf[i];
			if (hx <= 0xf)
				std::cout << std::hex << 0 << hx << " "; //leading 0
			else
				std::cout << std::hex << hx << " ";
		}
		std::cout << std::endl;
	}

	~joycon() {
		alive = false;

		if (callback_thread.joinable())
			callback_thread.join();
	}

	void capture() {
		hid_set_nonblocking(handle, 1);
		callback_thread = std::thread(&joycon::callback, this);
	}

	void callback() {
		std::vector<unsigned char> buf(65, 0);
		while (alive) {
			std::fill(buf.begin(), buf.end(), 0); //cleanup

			// Read requested state
			hid_read(handle, buf.data(), 65);

			// Print out the returned buffer.

			if (buf[0] == 0x0) {
				continue;
			}

			printBuf(buf, 32);
		}
	}

private:
	hid_device* handle;
	std::thread callback_thread;
	bool alive;
};

int main() {

	std::vector<joycon> joycons;

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
	} else {
		std::cout << "Starting capture for " << joycons.size() << " devices!" << std::endl;
		for (joycon& jc : joycons) {
			jc.capture();
		}
	}

	std::cin.get();

	// Finalize the hidapi library
	hid_exit();

}