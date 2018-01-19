#pragma once

#include <string>

#include "types.h"

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
