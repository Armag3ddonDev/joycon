#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <mutex>

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
		this->enable_vibration(true);

		std::cout << "Enabling IMU..." << std::endl;
		this->enable_IMU(true);

		std::cout << "Increasing data rate for Bluetooth..." << std::endl;
		this->set_input_report_mode(0x30);

		std::cout << "Reading SPI calibration..." << std::endl;
		// this->sensorCalib = this->get_sensor_calibration();

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

	std::lock_guard<std::mutex> lock(hid_mutex);

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

InputBuffer Joycon::send_command(unsigned char cmd, unsigned char subcmd, const ByteVector& data, bool blocking, Rumble rumble) {

	std::lock_guard<std::mutex> lock(hid_mutex);
	if (blocking) { CHECK(hid_set_nonblocking(handle, 0)); }

	OutputBuffer buff_out(data.size());
	buff_out.set_cmd(cmd);
	buff_out.set_subcmd(subcmd);
	buff_out.set_data(data);
	buff_out.set_rumble_left(rumble);
	buff_out.set_rumble_right(rumble);
	buff_out.set_GP(package_number & 0x0F);

	std::cout << "	sending : " << buff_out << std::endl;

	CHECK(hid_write(handle, buff_out.data(), buff_out.size()));

	InputBuffer buff_in;
	if (blocking) {
		CHECK(hid_read(handle, buff_in.data(), buff_in.size()));

		std::cout << "	received: " << buff_in << std::endl;

		CHECK(hid_set_nonblocking(handle, 1)); 
	}

	++package_number;

	return buff_in;
}

void Joycon::callback() {

	InputBuffer buff_in;
	while (alive) {
		buff_in.clean();

		// Read requested state
		{
			std::lock_guard<std::mutex> lock(hid_mutex);
			CHECK(hid_read(handle, buff_in.data(), buff_in.size()));
		}

		if (buff_in.get_ID() == 0x00) {
			continue;
		}

		std::cout << buff_in << std::endl;
	}
}

void Joycon::capture() {

	CHECK(hid_set_nonblocking(handle, 1));
	callback_thread = std::thread(&Joycon::callback, this);
}

JoyconDeviceInfo Joycon::request_device_info() {
	InputBuffer buff_in = this->send_command(0x01, 0x02, {}, true);

	JoyconDeviceInfo info;
	ByteVector data = buff_in.get_reply_data(0, 12);
	info.firmwareVersion = std::to_string(data[0]) + "." + std::to_string(data[1]);
	info.joyconType = data[2];
	info.mac = to_hex_string(data, 4, 6, "", ":");
	info.useColorsSPI = data[11];

	return info;
}

void Joycon::set_input_report_mode(unsigned char irm) {
	check_input_arguments({ 0x00, 0x01, 0x02, 0x23, 0x30, 0x31, 0x3F }, irm, "Invalid input-report-mode");

	unsigned char cmd{ 0x01 };
	// special cases
	if (irm == 0x00 || irm == 0x01 || irm == 0x02) {
		cmd = 0x11;
	}

	this->send_command(cmd, 0x03, { irm }, true);
}

TriggerButtonElapsedTime Joycon::trigger_button_elapsed_time() {
	InputBuffer buff_in = this->send_command(0x01, 0x04, {}, true);

	TriggerButtonElapsedTime res;
	ByteVector data = buff_in.get_reply_data(0, 14);
	res.L = std::chrono::milliseconds(to_int(data, 0, 2, false));
	res.R = std::chrono::milliseconds(to_int(data, 2, 2, false));
	res.ZL = std::chrono::milliseconds(to_int(data, 4, 2, false));
	res.ZR = std::chrono::milliseconds(to_int(data, 6, 2, false));
	res.SL = std::chrono::milliseconds(to_int(data, 8, 2, false));
	res.SR = std::chrono::milliseconds(to_int(data, 10, 2, false));
	res.HOME = std::chrono::milliseconds(to_int(data, 12, 2, false));

	return res;
}

void Joycon::set_HCI_state(unsigned char state) {
	check_input_arguments({ 0x00, 0x01, 0x02, 0x04 }, state, "Invalid input HCI-state");
	this->send_command(0x01, 0x06, { state }, true);
}

#ifdef ENABLE_UNTESTED
// Initializes the 0x2000 SPI section.
void Joycon::reset_pairing_info() {
	this->send_command(0x01, 0x07, {}, true);
}
#endif

#ifdef ENABLE_UNTESTED
void Joycon::set_shipment(bool enable) {
	this->send_command(0x01, 0x08, { static_cast<unsigned char>(enable) }, true);
}
#endif

ByteVector Joycon::SPI_flash_read(unsigned int address, unsigned char length) {

	if (address > 0xFFFFFFFF) {
		throw std::invalid_argument("Address can only have 4 byte!");
	}

	if (length > 0x1D) {
		throw std::runtime_error("length must be less than 0x1D.");
	}

	ByteVector data_send(5);
	to_byte_container(address, data_send, 0, 4, false);
	data_send[4] = length;

	InputBuffer buff_in = this->send_command(0x01, 0x10, data_send, true);

	if (buff_in.get_subcommandID_reply() != 0x90 || 
		to_int(buff_in.get_reply_data(0, 4), false) != address ||
		buff_in.get_reply_data_at(4) != length) 
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	return buff_in.get_reply_data(4);
}

#ifdef ENABLE_UNTESTED
void Joycon::SPI_flash_write(unsigned int address, ByteVector data) {

	if (address > 0xFFFFFFFF) {
		throw std::invalid_argument("Address can only have 4 byte!");
	}

	if (data.size() > 0x1D) {
		throw std::runtime_error("length must be less than 0x1D.");
	}

	ByteVector data_send(5 + data.size());
	to_byte_container(address, data_send, 0, 4, false);
	data_send[4] = data.size();

	// write data
	std::copy(data.begin(), data.end(), data_send.begin() + 5);

	InputBuffer buff_in = this->send_command(0x01, 0x11, data_send, true);

	if (buff_in.get_subcommandID_reply() != 0x91   ||
		to_int(buff_in.get_reply_data(0, 4), false) != address ||
		buff_in.get_reply_data_at(4) != length)
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	if (buff_in.get_reply_data_at(5) != 0x01) {
		throw std::runtime_error("SPI-write failed.")
	}
}
#endif

#ifdef ENABLE_UNTESTED
void Joycon::SPI_sector_erase(unsigned int address) {

	if (address > 0xFFFFFFFF) {
		throw std::invalid_argument("Address can only have 4 byte!");
	}

	InputBuffer buff_in = this->send_command(0x01, 0x12, address.swapped(), true);

	if (buff_in.get_subcommandID_reply() != 0x92 ||
		to_int(buff_in.get_reply_data(0, 4), false) != address)
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	if (buff_in.get_reply_data_at(4) != 0x00) {
		throw std::runtime_error("SPI-erase failed.")
	}
}
#endif

void Joycon::set_player_lights(PLAYER_LIGHTS arg) {
	this->send_command(0x01, 0x30, { static_cast<unsigned char>(arg) }, false);
}

PLAYER_LIGHTS Joycon::get_player_lights() {
	InputBuffer buff_in = this->send_command(0x01, 0x31, {}, true);

	if (buff_in.get_ACK() != 0xB0 || 
		buff_in.get_subcommandID_reply() != 0x31) 
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	return static_cast<PLAYER_LIGHTS>(buff_in.get_reply_data_at(0) & 0xFF);
}

void Joycon::set_home_light(const HOME_LIGHT& light_data) {
	InputBuffer buff_in = this->send_command(0x01, 0x38, light_data.data(), false);
}

void Joycon::enable_IMU(bool enable) {
	this->send_command(0x01, 0x40, { static_cast<unsigned char>(enable) }, true);
}

// Sending x40 x01 (IMU enable), if it was previously disabled, resets your configuration to 0x03 0x00 0x01 0x01
void Joycon::set_IMU_sensitivity(unsigned char gyro_sens, unsigned char acc_sens, unsigned char gyro_perf_rate, unsigned char acc_aa_filter) {
	check_input_arguments({ 0x00, 0x01, 0x02, 0x03 }, gyro_sens, "Invalid gyro_sens input");
	check_input_arguments({ 0x00, 0x01, 0x02, 0x03 }, acc_sens, "Invalid acc_sens input");
	check_input_arguments({ 0x00, 0x01 }, gyro_perf_rate, "Invalid gyro_perf_rate input");
	check_input_arguments({ 0x00, 0x01 }, acc_aa_filter, "Invalid acc_aa_filter input");

	this->send_command(0x01, 0x41, { gyro_sens, acc_sens, gyro_perf_rate, acc_aa_filter }, true);
}

#ifdef ENABLE_UNTESTED
void Joycon::write_IMU_register(unsigned char address, unsigned char value) {
	check_input_arguments({}, address, "Invalid address");	// <-- TODO
	this->send_command(0x01, 0x42, { address, 0x01, value }, true);
}
#endif

#ifdef ENABLE_UNTESTED
unsigned char Joycon::read_IMU_register(unsigned char address) {

	check_input_arguments({}, address, "Invalid address");	// <-- TODO

	InputBuffer buff_in = this->send_command(0x01, 0x43, { address, 0x01 }, true);

	if (buff_in.get_ACK() != 0xC0 || 
		buff_in.get_subcommandID_reply() != 0x43 || 
		buff_in.get_replay_data_at(0) != address ||
		buff_in.get_reply_data_at(1) != 0x01)
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	return buff_in.get_reply_data_at(2);
}
#endif

#ifdef ENABLE_UNTESTED
ByteVector Joycon::read_IMU_registers(unsigned char start_address, unsigned char amount) {

	check_input_arguments({}, start_address, "Invalid start_address");	// <-- TODO
	if (amount > 0x20) { throw std::invalid_argument("Max amount is 0x20."); }

	InputBuffer buff_in = this->send_command(0x01, 0x43, { start_address, amount }, true);

	if (buff_in.get_ACK() != 0xC0 ||
		buff_in.get_subcommandID_reply() != 0x43 ||
		buff_in.get_replay_data_at(0) != address ||
		buff_in.get_reply_data_at(1) != amount)
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	return buff_in.get_reply_data(2);
}
#endif

void Joycon::enable_vibration(bool enable) {
	this->send_command(0x01, 0x48, { static_cast<unsigned char>(enable) }, false);
}

POWER Joycon::get_regulated_voltage() {
	InputBuffer buff_in = this->send_command(0x01, 0x50, {}, true);

	if (buff_in.get_ACK() != 0xD0 ||
		buff_in.get_subcommandID_reply() == 0x50)
	{
		throw std::runtime_error("Did not receive correct answer!");
	}

	unsigned long int power_level = to_int(buff_in.get_reply_data(0, 2), false);
	if (power_level <= 0x059F) { return POWER::CRITICAL; }
	else if (power_level <= 0x05DF) { return POWER::LOW; }
	else if (power_level <= 0x0617) { return POWER::MEDIUM; }
	else { return POWER::FULL; }
}

void Joycon::send_rumble(Rumble rumble) {
	this->send_command(0x10, 0x00, {}, false, rumble);
}

SensorCalibration Joycon::get_sensor_calibration() {
	SensorCalibration calib;
	calib.factory_sensor_cal	= this->SPI_flash_read(0x6020, 0x18);
	calib.factory_stick_cal		= this->SPI_flash_read(0x603D, 0x12);
	calib.sensor_model			= this->SPI_flash_read(0x6080, 0x06);
	calib.stick_model1			= this->SPI_flash_read(0x6086, 0x12);
	calib.stick_model2			= this->SPI_flash_read(0x6098, 0x12);
	calib.user_stick_cal		= this->SPI_flash_read(0x8010, 0x16);
	calib.user_sensor_cal		= this->SPI_flash_read(0x8026, 0x1A);

	return calib;
}

Color24 Joycon::get_body_RGB() {

	Color24 res;
	ByteVector reply = this->SPI_flash_read(0x6050, 0x03);

	res.R = reply.at(0);
	res.G = reply.at(1);
	res.B = reply.at(2);

	return res;
}

Color24 Joycon::get_button_RGB() {

	Color24 res;
	ByteVector reply = this->SPI_flash_read(0x6053, 0x03);

	res.R = reply.at(0);
	res.G = reply.at(1);
	res.B = reply.at(2);

	return res;
}

void Joycon::check_input_arguments(std::unordered_set<unsigned char> list, unsigned char arg, std::string error_msg) const {
	if (list.find(arg) == list.end()) {
		std::stringstream error;
		error << error_msg << ": " << std::hex << static_cast<unsigned int>(arg);
		throw std::invalid_argument(error.str());
	}
}

/* ------ JOYCONVEC ------ */

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
			vec.emplace_back(new Joycon(static_cast<JOY_PID>(current->product_id), current->serial_number));
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

	if (vec.size() == 0) {
		std::cout << "No joy-con device detected!" << std::endl;
		return -1;
	} else {
		std::cout << "Starting capture for " << vec.size() << " devices!" << std::endl;
		for (const auto& jc_ptr : vec) {
			jc_ptr->capture();
		}
	}
	return 0;
}
