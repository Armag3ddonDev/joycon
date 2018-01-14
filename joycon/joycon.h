#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include "hidapi.h"
#elif __linux__
#include <hidapi/hidapi.h>
#endif

#include "buffer.h"

#define THROW(x) throw(std::runtime_error(std::string(__FILE__) + " - line " + std::to_string(__LINE__) + ": " + __FUNCTION__ + "(): " + x ))
#define CHECK(x) if (x == -1) {THROW(#x + " failed!");}

#define MAX_STR 255
#define JOYCON_VENDOR 0x057e

// #define ENABLE_UNTESTED

enum JOY_PID {
	JOYCON_L_BT = 0x2006,
	JOYCON_R_BT = 0x2007,
	PRO_CONTROLLER = 0x2009,
	JOYCON_CHARGING_GRIP = 0x200e
};

struct JoyconDeviceInfo {
	std::string firmwareVersion;	// Firmware Version. Latest is 3.86 (from 4.0.0 and up).
	unsigned int joyconType;		// 1=Left Joy-Con, 2=Right Joy-Con, 3=Pro Controller
	std::string mac;				// Joy-Con MAC address in Big Endian
	bool useColorsSPI;				// If true, colors in SPI are used. Otherwise default ones.
};

struct TriggerButtonElapsedTime {
	std::chrono::milliseconds L, R, ZL, ZR, SL, SR, HOME;
};

// On overrides flashing. When on USB, flashing bits work like always on bits.
enum PLAYER_LIGHTS {
	P0_KEEP_ON = 1 << 0,
	P1_KEEP_ON = 1 << 1,
	P2_KEEP_ON = 1 << 2,
	P3_KEEP_ON = 1 << 3,
	P0_FLASH = 1 << 4,
	P1_FLASH = 1 << 5,
	P2_FLASH = 1 << 6,
	P3_FLASH = 1 << 7
};

constexpr PLAYER_LIGHTS operator|(PLAYER_LIGHTS X, PLAYER_LIGHTS Y) {
	return static_cast<PLAYER_LIGHTS>(static_cast<unsigned char>(X) | static_cast<unsigned char>(Y));
}

constexpr PLAYER_LIGHTS operator&(PLAYER_LIGHTS X, PLAYER_LIGHTS Y) {
	return static_cast<PLAYER_LIGHTS>(static_cast<unsigned char>(X) & static_cast<unsigned char>(Y));
}

class Joycon {

public:
	Joycon(Joycon&) = delete;
	Joycon(Joycon&&) = default;
	Joycon(JOY_PID PID, wchar_t* serial_number);

	~Joycon();

	void printDeviceInfo() const;
	InputBuffer send_command(unsigned char cmd, unsigned char subcmd, std::vector<unsigned char> data, bool blocking = true);
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

	ByteVector SPI_flash_read();

#ifdef ENABLE_UNTESTED
	void SPI_flash_write();
#endif

#ifdef ENABLE_UNTESTED
	void SPI_sector_erase();
#endif

	void set_player_lights(PLAYER_LIGHTS arg);

	PLAYER_LIGHTS get_player_lights();

	void set_home_light();

	void enable_IMU(bool enable);

	// Sending x40 x01 (IMU enable), if it was previously disabled, resets your configuration to 0x03 0x00 0x01 0x01
	void set_IMU_sensitivity(unsigned char gyro_sens, unsigned char acc_sens, unsigned char gyro_perf_rate, unsigned char acc_aa_filter);

#ifdef ENABLE_UNTESTED
	void write_IMU_register(unsigned char address, unsigned char value);
#endif

	unsigned char read_IMU_register(unsigned char address);
	ByteVector read_IMU_registers(unsigned char start_address, unsigned char amount);

	void enable_vibration(bool enable);

	POWER get_regulated_voltage();

private:

	void check_input_arguments(std::unordered_set<unsigned char> list, unsigned char arg, std::string error_msg) const;

	hid_device* handle;
	std::thread callback_thread;
	bool alive = true;
	std::size_t package_number = 0;
};

class JoyconVec {
public:
	int addDevices();
	int startDevices();
private:
	std::vector<Joycon> vec;
};
