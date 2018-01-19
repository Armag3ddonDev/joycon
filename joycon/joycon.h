#pragma once

#include <mutex>
#include <string>
#include <stdexcept>
#include <thread>
#include <vector>

#ifdef _WIN32
#include "hidapi.h"
#elif __linux__
#include <hidapi/hidapi.h>
#endif

#include "buffer.h"
#include "homelight.h"

#define THROW(x) throw(std::runtime_error(std::string(__FILE__) + " - line " + std::to_string(__LINE__) + ": " + __FUNCTION__ + "(): " + x ))
#define CHECK(x) if (x == -1) {THROW(#x + " failed!");}

#define MAX_STR 255
#define JOYCON_VENDOR 0x057e

// #define ENABLE_UNTESTED

class Joycon {

public:
	Joycon(Joycon&) = delete;
	Joycon(Joycon&&) = delete;
	Joycon(JOY_PID PID, wchar_t* serial_number);

	~Joycon();

	void printDeviceInfo() const;
	InputBuffer send_command(unsigned char cmd, unsigned char subcmd, const ByteVector& data, bool blocking = true, Rumble rumble = Rumble());
	void capture();
	void callback();

	JoyconDeviceInfo request_device_info();

	// 0x00 - Used with cmd x11.Active polling for IR camera data. 0x31 data format must be set first
	// 0x01 - Same as 00
	// 0x02 - Same as 00. Active polling mode for IR camera data.For specific IR modes
	// 0x23 - MCU update state report ?
	// 0x30 - Standard full mode.Pushes current state @60Hz
	// 0x31 - NFC / IR mode.Pushes large packets @60Hz
	// 0x33 - Unknown mode.
	// 0x35 - Unknown mode.
	// 0x3F - Simple HID mode.Pushes updates with every button press
	// 31 input report has all zeroes for IR/NFC data if a 11 ouput report with subcmd 03 00 or 03 01 or 03 02 was not sent before.
	void set_input_report_mode(unsigned char irm);

	TriggerButtonElapsedTime trigger_button_elapsed_time();

	// x00 	Disconnect(sleep mode / page scan mode)
	// x01 	Reboot and Reconnect(page mode)
	// x02 	Reboot and enter Pair mode(discoverable)
	// x04 	Reboot and Reconnect(page mode / HOME mode ? )
	void set_HCI_state(unsigned char state);

	// Initializes the 0x2000 SPI section.
#ifdef ENABLE_UNTESTED
	void reset_pairing_info();
#endif

#ifdef ENABLE_UNTESTED
	void set_shipment(bool enable);
#endif

	ByteVector SPI_flash_read(ByteVector address, unsigned char length);

#ifdef ENABLE_UNTESTED
	void SPI_flash_write(ByteVector address, ByteVector data);
#endif

#ifdef ENABLE_UNTESTED
	void SPI_sector_erase(ByteVector address);
#endif

	void set_player_lights(PLAYER_LIGHTS arg);

	PLAYER_LIGHTS get_player_lights();

	void set_home_light(const HOME_LIGHT& led_data);

	void enable_IMU(bool enable);

	// Sending x40 x01 (IMU enable), if it was previously disabled, resets your configuration to 0x03 0x00 0x01 0x01
	void set_IMU_sensitivity(unsigned char gyro_sens, unsigned char acc_sens, unsigned char gyro_perf_rate, unsigned char acc_aa_filter);

#ifdef ENABLE_UNTESTED
	void write_IMU_register(unsigned char address, unsigned char value);
	unsigned char read_IMU_register(unsigned char address);
	ByteVector read_IMU_registers(unsigned char start_address, unsigned char amount);
#endif

	void enable_vibration(bool enable);

	POWER get_regulated_voltage();

	void send_rumble(Rumble rumble = Rumble());

private:

	void check_input_arguments(std::unordered_set<unsigned char> list, unsigned char arg, std::string error_msg) const;

	hid_device* handle;
	std::thread callback_thread;
	bool alive = true;
	std::size_t package_number = 0;

	mutable std::mutex hid_mutex;
};

class JoyconVec {
public:
	int addDevices();
	int startDevices();

	std::size_t size() { return vec.size(); }
	Joycon& device(std::size_t idx) { return *vec.at(idx); }
private:
	std::vector<std::unique_ptr<Joycon>> vec;
};
