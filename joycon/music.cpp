#include <chrono>
#include <iostream>
#include <signal.h>
#include <thread>
#include <vector>

#ifdef _WIN32
#include "hidapi.h"
#elif __linux__
#include <hidapi/hidapi.h>
#endif

#include "joycon.h"
#include "rumble.h"

static sig_atomic_t volatile shutdown_flag = 0;
static void SigCallback(int sig) {
	shutdown_flag = 1;
}

struct Note {
	Note(Rumble Note, double duration) : rumble(rumble), duration(duration) {}
	Rumble rumble;
	double duration;
};

using Song = std::vector<Note>;

// all notes with safe frequencies for the rumble command
namespace Music {
	static constexpr double Eb6	=	1244.51;
	static constexpr double Ds6	=	1244.51;
	static constexpr double D6	=	1174.66;
	static constexpr double Db6	=	1108.73;
	static constexpr double Cs6	=	1108.73;
	static constexpr double C6	=	1046.50;
	static constexpr double B5	=	987.767;
	static constexpr double Bb5	=	932.328;
	static constexpr double As5	=	932.328;
	static constexpr double A5	=	880.000;
	static constexpr double Ab5	=	830.609;
	static constexpr double Gs5	=	830.609;
	static constexpr double G5	=	783.991;
	static constexpr double Gb5	=	739.989;
	static constexpr double Fs5	=	739.989;
	static constexpr double F5	=	698.456;
	static constexpr double E5	=	659.255;
	static constexpr double Eb5	=	622.254;
	static constexpr double Ds5	=	622.254;
	static constexpr double D5	=	587.330;
	static constexpr double Db5	=	554.365;
	static constexpr double Cs5	=	554.365;
	static constexpr double C5	=	523.251;
	static constexpr double B4	=	493.883;
	static constexpr double Bb4	=	466.164;
	static constexpr double As4	=	466.164;
	static constexpr double A4	=	440.000;
	static constexpr double Ab4	=	415.305;
	static constexpr double Gs4	=	415.305;
	static constexpr double G4	=	391.995;
	static constexpr double Gb4	=	369.994;
	static constexpr double Fs4	=	369.994;
	static constexpr double F4	=	349.228;
	static constexpr double E4	=	329.628;
	static constexpr double Eb4	=	311.127;
	static constexpr double Ds4	=	311.127;
	static constexpr double D4	=	293.665;
	static constexpr double Db4	=	277.183;
	static constexpr double Cs4	=	277.183;
	static constexpr double C4	=	261.626;
	static constexpr double B3	=	246.942;
	static constexpr double Bb3	=	233.082;
	static constexpr double As3	=	233.082;
	static constexpr double A3	=	220.000;
	static constexpr double Ab3	=	207.652;
	static constexpr double Gs3	=	207.652;
	static constexpr double G3	=	195.998;
	static constexpr double Gb3	=	184.997;
	static constexpr double Fs3	=	184.997;
	static constexpr double F3	=	174.614;
	static constexpr double E3	=	164.814;
	static constexpr double Eb3	=	155.563;
	static constexpr double Ds3	=	155.563;
	static constexpr double D3	=	146.832;
	static constexpr double Db3	=	138.591;
	static constexpr double Cs3	=	138.591;
	static constexpr double C3	=	130.813;
	static constexpr double B2	=	123.471;
	static constexpr double Bb2	=	116.541;
	static constexpr double As2	=	116.541;
	static constexpr double A2	=	110.000;
	static constexpr double Ab2	=	103.826;
	static constexpr double Gs2	=	103.826;
	static constexpr double G2	=	97.9989;
	static constexpr double Gb2	=	92.4986;
	static constexpr double Fs2	=	92.4986;
	static constexpr double F2	=	87.3071;
	static constexpr double E2	=	82.4069;
	static constexpr double Eb2	=	77.7817;
	static constexpr double Ds2	=	77.7817;
	static constexpr double D2	=	73.4162;
	static constexpr double Db2	=	69.2957;
	static constexpr double Cs2	=	69.2957;
	static constexpr double C2	=	65.4064;
	static constexpr double B1	=	61.7354;
	static constexpr double Bb1	=	58.2705;
	static constexpr double As1	=	58.2705;
	static constexpr double A1	=	55.0000;
	static constexpr double Ab1	=	51.9131;
	static constexpr double Gs1	=	51.9131;
	static constexpr double G1	=	48.9994;
	static constexpr double Gb1	=	46.2493;
	static constexpr double Fs1	=	46.2493;
	static constexpr double F1	=	43.6535;
	static constexpr double E1	=	41.2034;

	static constexpr double P = 1200;


	Song TTTE = {
	Note(Rumble(G3,	1.0),	1./8.),
	Note(Rumble(A3,	1.0),	1./8.),
	Note(Rumble(B3,	1.0),	1./8.),
	Note(Rumble(C4,	1.0),	1./4.),
	Note(Rumble(D4,	1.0),	1./8.),
	Note(Rumble(E4,	1.0),	1./8.),
	Note(Rumble(P,	0.0),	1./8.),

	Note(Rumble(Ab3,	1.0),	1.),

	Note(Rumble(A3,	1.0),	1./8.),
	Note(Rumble(F3,	1.0),	1./8.),
	Note(Rumble(A3,	1.0),	1./8.),
	Note(Rumble(G3,	1.0),	1./8. + 1./4.),
	Note(Rumble(P,	0.0),	1./8. + 1./16.),
	Note(Rumble(Gs3,	1.0),	1./16.),

	Note(Rumble(A3,	1.0),	1./8.),
	Note(Rumble(F3,	1.0),	1./8.),
	Note(Rumble(F3,	1.0),	1./8.),
	Note(Rumble(A3,	1.0),	1./16.),
	Note(Rumble(G3,	1.0),	1./16. + 1./8. + 1./16.),
	Note(Rumble(Fs3,	1.0),	1./16.),
	Note(Rumble(G3,	1.0),	1./16.),
	Note(Rumble(Fs3,	1.0),	1./16.),
	Note(Rumble(G3,	1.0),	1./16.),
	Note(Rumble(Fs3,	1.0),	1./16.),

	Note(Rumble(G3,	1.0),	1./4.),
	Note(Rumble(G3,	1.0),	1./8),
	Note(Rumble(P,	0.0),	1./4. + 1./16.),
	Note(Rumble(Fs3,	1.0),	1./16.),
	Note(Rumble(G3,	1.0),	1./16.),
	Note(Rumble(Fs3,	1.0),	1./16.),
	Note(Rumble(G3,	1.0),	1./8.),

	Note(Rumble(A3,	1.0),	1./4.),
	Note(Rumble(A3,	1.0),	1./8.),
	Note(Rumble(P,	0.0),	1./8. + 1./16.),
	Note(Rumble(Eb3,	1.0),	1./16.),
	Note(Rumble(Eb3,	1.0),	1./8.),
	Note(Rumble(F3,	1.0),	1./8.),
	Note(Rumble(Fs3,	1.0),	1./8.),

	Note(Rumble(G3,	1.0),	1./2.),
	Note(Rumble(Bb3,	1.0),	1./2.),
	Note(Rumble(F3,	1.0),	1./2.),
	Note(Rumble(G3,	1.0),	1./2.),

	Note(Rumble(A3,	1.0),	1./16.)};

}

int main() {
	std::ios_base::sync_with_stdio(false);

	// Initialize the hidapi library
	if (hid_init()) {
		std::cerr << "HID initialization failed!" << std::endl;
		return -1;
	}

	signal(SIGINT, SigCallback);
	signal(SIGTERM, SigCallback);

	JoyconVec joycons;
	if (joycons.addDevices() == -1) { hid_exit();  return 0; }
	if (joycons.startDevices() == -1) { hid_exit();  return 0; };

	for (const Note& note : Music::TTTE) {

		joycons.device(0).send_rumble(note.rumble);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned int>(note.duration*1000)));
	}

	while (!shutdown_flag) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	hid_exit();
	return 0;
}