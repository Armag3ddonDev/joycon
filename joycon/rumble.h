#pragma once

#include <string>

#include "types.h"

// data: 01LL LLLL | KIII IIII | JJJJ JJJG | HHHH HH00

/* ORIGINAL */
// ID		START	STEP	END			Byte
// hf:		0x0004	0x0004	0x01FC		0000 000G HHHH HH00
// lf:		0x01	0x01	0x7F		          0III IIII
// hf_amp:	0x00	0x02	0xFE		          JJJJ JJJ0
// lf_amp:	0xX040	0x0001	0xX07F		K000 0000 01LL LLLL		// where X alternates between 8 and 0

/* ALTERNATIVE <--- USED */ 
// ID		START	STEP	END		Byte
// hf:		0x01	0x01	0x7F	0GHH HHHH
// lf:		0x01	0x01	0x7F	0III IIII
// hf_amp:	0x01	0x01	0x7F	0JJJ JJJJ
// lf_amp:	0x01	0x01	0x7F	0LLL LLLK
// -> valid if any < 0x80

// data[0] = (hf << 2);
// data[1] = (hf_amp << 1) | (hf >> 6);
// data[2] = (lf_amp << 7) | lf;
// data[3] = 0x40 | (lf_amp >> 1);

// hf     = (data[1] << 6) | (data[0] >> 2);
// hf_amp = data[1] >> 1;
// lf     = (data[2] << 1) >> 1;
// lf_amp = ((data[3] << 2) >> 1) | (data[2] >> 7);

class Rumble
{
public:

	Rumble() {}

	Rumble(double frequency, double amplitude) : frequency(frequency), amplitude(amplitude) {
		pack();
	}

	Rumble(const ByteArray<4>& data) : frequency(0.0), amplitude(0.0),  data(data) {
		unpack();
	}

	const double& getFreqeuncy() const { return frequency; }
	const double& getAmplitude() const { return amplitude; }
	const byte& byte_at(std::size_t idx) const { return data.at(idx); }

private:

	void pack();	// frequency + amplitude -> data (4 byte)
	void unpack();	// frequency + amplitude <- data (4 byte)

	void encode_frequency(double frequency, byte& hf, byte& lf) const;
	void encode_amplitude(double amplitude, byte& hf_amp, byte& lf_amp) const;

	double decode_frequency(byte hf, byte lf) const;
	double decode_amplitude(byte hf_amp, byte lf_amp) const;

	double frequency;
	double amplitude;
	ByteArray<4> data{0x00, 0x01, 0x40, 0x40};
};
