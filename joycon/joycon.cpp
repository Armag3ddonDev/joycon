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
		THROW("Constructor failed to initialize: " + e.what());
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
	//CHECK(hid_get_indexed_string(handle, 1, wstr, MAX_STR));
	//std::wcout << L"	Indexed String 1: " << wstr << std::endl;
}

void Joycon::send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data) {

	CHECK(hid_set_nonblocking(handle, 0));

	OutputBuffer buf_out;
	buf_out.set_cmd(cmd);
	buf_out.set_subcmd(subcmd);
	buf_out.set_data(data);
	buf_out.set_GP(package_number & 0x0F);

	std::cout << "sending:  ";
	buf_out.printBuf();

	CHECK(hid_write(handle, buf_out.data(), buf_out.size()));

	InputBuffer buf_in;
	CHECK(hid_read(handle, buf_in.data(), buf_in.size()));

	std::cout << "received: ";
	buf_in.printBuf();

	CHECK(hid_set_nonblocking(handle, 1));

	++package_number;
}

void Joycon::callback() {

	InputBuffer buf_in;
	while (alive) {
		buf_in.clean();

		// Read requested state
		CHECK(hid_read(handle, buf_in.data(), buf_in.size()));

		if (buf_in[0] == 0x00) {
			continue;
		}

		processReply(buf_in);

		buf_in.printBuf();
	}
}

void Joycon::capture() {

	CHECK(hid_set_nonblocking(handle, 1));
	callback_thread = std::thread(&Joycon::callback, this);
}

void Joycon::processReply(InputBuffer& buf_in) {
	auto cmd = buf_in.cmd();
	switch (cmd) {
	case 0x30: //
		//accel
		std::cout << std::dec << ((buf_in[16] << 8) + buf_in[15]) * 0.000244f << std::endl;
		std::cout << std::dec << ((buf_in[18] << 8) + buf_in[17]) * 0.000244f << std::endl;
		std::cout << std::dec << ((buf_in[20] << 8) + buf_in[19]) * 0.000244f << std::endl;
		//gyro
		std::cout << std::dec << ((buf_in[22] << 8) + buf_in[21]) * 0.070f << std::endl;
		std::cout << std::dec << ((buf_in[24] << 8) + buf_in[23]) * 0.070f << std::endl;
		std::cout << std::dec << ((buf_in[26] << 8) + buf_in[25]) * 0.070f << std::endl;
		break;
	default:
		break;
	}
}

int JoyconVec::addDevices() {

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

int JoyconVec::startDevices() {

	if (this->size() == 0) {
		std::cout << "No joy-con device detected!" << std::endl;
		return -1;
	} else {
		std::cout << "Starting capture for " << this->size() << " devices!" << std::endl;
		for (Joycon& jc : *this) {
			jc.capture();
		}
	}
	return 0;
}
