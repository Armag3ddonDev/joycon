#include <iostream>

#include "joycon.h"
#include "buffer.h"

Joycon::Joycon(JOY_PID PID, wchar_t* serial_number) : package_number(0) {
	
	std::cout << "Adding device:" << std::endl;
	std::cout << "PID: " << std::hex << PID << std::endl;
	std::wcout << L"SN : " << serial_number << std::endl;

	handle = hid_open(JOYCON_VENDOR, PID, serial_number);

	if(handle == nullptr) {
		std::string error("");
		error += "Handle could not be set!\n";
		error += "You need to run this program as sudo maybe?"; //TODO FIX THIS!!!
		THROW(error);
	}

	try {
		this->printDeviceInfo();

		std::cout << "Enabling vibration..." << std::endl;
		this->send_command(0x01, 0x48, { 0x01 });

		std::cout << "Enabling IMU..." << std::endl;
		this->send_command(0x01, 0x40, { 0x01 });

		std::cout << "Increasing data rate for Bluetooth..." << std::endl;
		this->send_command(0x01, 0x03, { 0x30 });
	} catch (std::exception& e) {
		hid_close(handle);
		THROW("Constructor failed to initialize.");
	}
}

Joycon::~Joycon() {
	alive = false;

	if (callback_thread.joinable())
		callback_thread.join();

	hid_close(handle);
}

void Joycon::printDeviceInfo() const {
	
	std::cout << "Device Info:" << std::endl;
	
	wchar_t wstr[MAX_STR];

	// Read the Manufacturer String
	CHECK(hid_get_manufacturer_string(handle, wstr, MAX_STR));
	std::wcout << L"	Manufacturer String: " << wstr << std::endl;

	// Read the Product String
	CHECK(hid_get_product_string(handle, wstr, MAX_STR));
	std::wcout << L"	Product String: " << wstr << std::endl;

	// Read the Serial Number String
	CHECK(hid_get_serial_number_string(handle, wstr, MAX_STR));
	std::wcout << L"	Serial Number String: (" << wstr[0] << ") " << wstr << std::endl;

	// Read Indexed String 1
	CHECK(hid_get_indexed_string(handle, 1, wstr, MAX_STR));
	std::wcout << L"	Indexed String 1: " << wstr << std::endl;
}

void Joycon::send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data) {

	CHECK(hid_set_nonblocking(handle, 0));

	InputBuffer buf_in;
	buf_in.set_cmd(cmd);
	buf_in.set_subcmd(subcmd);
	buf_in.set_data(data);
	buf_in.set_GP(package_number & 0x0F);

	std::cout << "sending:  ";
	buf_in.printBuf(65);

	CHECK(hid_write(handle, buf_in.data(), buf_in.size()));

	OutputBuffer buf_out;
	CHECK(hid_read(handle, buf_out.data(), buf_out.size()));

	std::cout << "received: ";
	buf_out.printBuf(65);

	CHECK(hid_set_nonblocking(handle, 1));

	++package_number;
}


void Joycon::callback() {
	OutputBuffer buf_out;
	while (alive) {
		buf_out.clean();

		// Read requested state
		CHECK(hid_read(handle, buf_out.data(), buf_out.size()));

		if (buf_out[0] == 0x00) {
			continue;
		}

		buf_out.printBuf(65);
	}
}

void Joycon::capture() {
	CHECK(hid_set_nonblocking(handle, 1));
	callback_thread = std::thread(&Joycon::callback, this);
}

int JoyconVec::addDevices()
{

	std::cout << "Searching for devices..." << std::endl;

	hid_device_info* devs = hid_enumerate(JOYCON_VENDOR, 0x0);
	if (devs == nullptr) {
		std::cout << "No bluetooth device detected!" << std::endl;
		return -1;
	}

	std::cout << "-----------------------------" << std::endl;
	for (hid_device_info* current = devs; current != nullptr; current = current->next) {

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

		try {
			this->emplace_back(static_cast<JOY_PID>(current->product_id), current->serial_number);
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}

		std::cout << "-----------------------------" << std::endl;
	}

	hid_free_enumeration(devs);

	return 0;
}

int JoyconVec::startDevices()
{
	if (this->size() == 0) {
		std::cout << "No joy-con device detected!" << std::endl;
		return -1;
	}
	else {
		std::cout << "Starting capture for " << this->size() << " devices!" << std::endl;
		for (Joycon& jc : *this) {
			jc.capture();
		}
	}
	return 0;
}