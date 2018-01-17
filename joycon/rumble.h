#pragma once

#include "buffer.h"

class Rumble
{
public:

	Rumble() : frequency(0.0), amplitude(0.0), data({0x00, 0x01, 0x40, 0x40}) {
	}

	Rumble(double freqeuncy, double amplitude) : frequency(frequency), amplitude(amplitude), data() {
		pack();
	}

	Rumble(ByteArray<4> data) : frequency(0.0), amplitude(0.0),  data(data) {
		unpack();
	}

	const double& getFreqeuncy() const { return frequency; }
	const double& getAmplitude() const { return amplitude; }
	const unsigned char& byte_at(std::size_t idx) const { return data.at(idx); }

private:

	void pack();	// frequency + amplitude -> data (4 byte)
	void unpack();	// frequency + amplitude <- data (4 byte)

	void encode_frequency(double frequency, uint16_t& hf, uint8_t& lf) const;
	void encode_amplitude(double amplitude, uint8_t& hf_amp, uint16_t& lf_amp) const;

	double decode_frequency(uint16_t hf, uint8_t lf) const;
	double decode_amplitude(uint8_t hf_amp, uint16_t lf_amp) const;

	double frequency;
	double amplitude;
	ByteArray<4> data;
};

