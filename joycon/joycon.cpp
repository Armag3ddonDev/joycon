#include <iostream>
#include <vector>
#include <thread>

#include "joycon.h"
#include "buffer.h"

Joycon::Joycon(JOY_TYPE type, wchar_t* serial_number) {
	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(JOYCON_VENDOR, type, serial_number);

	if(handle == nullptr) {
		std::cerr << std::hex << type << std::endl;
		std::wcerr << serial_number << std::endl;
		std::cerr << "Handle couldn't be set!" << std::endl;
		std::cerr << "You need to run this program as sudo maybe?" << std::endl; //TODO FIX THIS!!!
		throw;
	}

	std::vector<unsigned char> buf(65, 0);

	wchar_t wstr[MAX_STR];
	int res;

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	std::wcout << L"Manufacturer String: " << wstr << std::endl;

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	std::wcout << L"Product String: " << wstr << std::endl;

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	std::wcout << L"Serial Number String: (" << wstr[0] << ") " << wstr << std::endl;

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	std::wcout << L"Indexed String 1: " << wstr << std::endl;

	std::cout << " >> Press Enter to continue << ";
	std::cin.get();

	unsigned int timing_byte = 0;

	std::cout << "Enabling vibration..." << std::endl;
	this->send_command(0x01, 0x48, { 0x01 });

	std::cout << "Enabling IMU..." << std::endl;
	this->send_command(0x01, 0x40, { 0x01 });

	std::cout << "Increasing data rate for Bluetooth..." << std::endl;
	this->send_command(0x01, 0x03, { 0x30 });

	std::cout << " >> Press Enter to continue << ";
	std::cin.get();
}

Joycon::~Joycon() {
	alive = false;

	if (callback_thread.joinable())
		callback_thread.join();
}

void Joycon::send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data) {

	int res = hid_set_nonblocking(handle, 0);
	if(res == -1) {
		std::cerr << "Could not set nonblocking mode!" << std::endl;
		throw;
	} else {
		std::cout << "Blocking mode set" << std::endl;
	}

	InputBuffer buf_in;
	buf_in.set_cmd(cmd);
	buf_in.set_subcmd(subcmd);
	buf_in.set_data(data);
	buf_in.set_GP(package_number & 0x0F);

	std::cout << "sending:  ";
	buf_in.printBuf(65);

	res = hid_write(handle, buf_in.data(), buf_in.size());
	if(res == -1) {
		std::cerr << "Write failed!" << std::endl;
		throw;
	} else {
		std::cout << res << std::endl;
	}

	OutputBuffer buf_out;
	if (hid_read(handle, buf_out.data(), buf_out.size()) < 0) {
		std::cout << "WARNING: read failed!" << std::endl;
	}

	std::cout << "received: ";
	buf_out.printBuf(65);

	++package_number;
}


void Joycon::callback() {
	OutputBuffer buf_out;
	while (alive) {
		buf_out.clean();

		// Read requested state
		hid_read(handle, buf_out.data(), buf_out.size());

		if (buf_out[0] == 0x00) {
			continue;
		}

		buf_out.printBuf(65);
	}
}

void Joycon::capture() {
	hid_set_nonblocking(handle, 1);
	callback_thread = std::thread(&Joycon::callback, this);
}
