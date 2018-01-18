#include <cmath>

#include "types.h"
#include "rumble.h"

void Rumble::pack() {

	byte hf;
	byte lf;
	byte hf_amp;
	byte lf_amp;

	encode_frequency(this->frequency, hf, lf);
	encode_amplitude(this->amplitude, hf_amp, lf_amp);

	this->data[0] = (hf << 2);
	this->data[1] = (hf_amp << 1) | (hf >> 6);
	this->data[2] = (lf_amp << 7) | lf;
	this->data[3] = 0x40 | (lf_amp >> 1);
}

void Rumble::unpack() {
	byte hf     = (data[1] << 6) | (data[0] >> 2);
	byte hf_amp = data[1] >> 1;
	byte lf     = (data[2] << 1) >> 1;
	byte lf_amp = ((data[3] << 2) >> 1) | (data[2] >> 7);

	this->frequency = decode_frequency(hf, lf);
	this->amplitude = decode_amplitude(hf_amp, lf_amp);
}

void Rumble::encode_frequency(double frequency, byte& hf, byte& lf) const {

	// Float frequency to hex conversion
	if (frequency < 40.87 || frequency > 1252.57) {
		throw std::invalid_argument("frequency must be between 40.87 and 1252.57.");
	}

	// maps to 0x41(65) - 0xDF(223)
	byte encoded_hex_freq = static_cast<byte>(std::round(std::log2(frequency / 10.0)*32.0));

	// Convert to Joy-Con HF range. Range: 0x01-0x7F
	hf = (encoded_hex_freq > 0x60) ? (encoded_hex_freq - 0x60) : 0x00;	// what happens under 81.75Hz?

	// Convert to Joy-Con LF range. Range: 0x01-0x7F.
	lf = (encoded_hex_freq < 0xC0) ? encoded_hex_freq - 0x40 : 0x00;
}

void Rumble::encode_amplitude(double amplitude, byte& hf_amp, byte& lf_amp) const {

	// dont increase for safety-reasons of the linear resonant actuators. max safe is 1.002867; real max is 1.799701
	if ((amplitude < 0.0) || (amplitude > 1.0)) {
		throw std::invalid_argument("amplitude must be between 0 and 1.");
	}

	// maps to 0x00(0) - 0x64(100) [safe] | 0x7E(126) [unsafe]
	byte amp_encoded;
	if (amplitude < 0.008) {
		amp_encoded = 0x00;
	}
	else if (amplitude < 0.112491) {
		amp_encoded = static_cast<uint8_t>(std::round((std::log2(amplitude * 5.0 / 18.0) + 9.0)*4.0)) - 1;
	}
	else if (amplitude < 0.224982) {
		amp_encoded = static_cast<uint8_t>(std::round((std::log2(amplitude * 5.0 / 18.0) + 6.0)*16.0)) - 1;
	}
	else {
		amp_encoded = static_cast<uint8_t>(std::round((std::log2(amplitude * 5.0 / 18.0) + 5.0)*32.0)) - 1;
	}

	// saveguard
	if (amp_encoded > 0x64) {
		throw std::bad_exception();
	}

	hf_amp = amp_encoded;
	lf_amp = amp_encoded;
}

double Rumble::decode_frequency(byte hf, byte lf) const {

	if (lf > 0x7F || hf > 0x7F) {
		throw std::invalid_argument("hf and lf must be less than 0x7F.");
	}

	byte encoded_hex_freq;

	if (lf != 0) {
		if (hf != 0 && lf + 0x20 != hf) {
			throw std::invalid_argument("lf (" + std::to_string(lf) + ") and hf (" + std::to_string(hf) + ") can not be equal!");
		} else {
			encoded_hex_freq = lf + 40;
		}
	} else if (hf == 0) {
			throw std::invalid_argument("lf and hf can not be both 0x00.");
	} else {
		encoded_hex_freq = hf + 60;
	}

	double frequency =  10.0*std::pow(2.0, static_cast<float>(encoded_hex_freq) / 32.0);
	return std::min(40.87, std::min(1252.57, frequency));
}

double Rumble::decode_amplitude(byte hf_amp, byte lf_amp) const {

	if (hf_amp != lf_amp) {
		throw std::runtime_error("lf_amp (" + std::to_string(lf_amp) + ") and hf_amp (" + std::to_string(hf_amp) + ") produce different frequencies!");;
	} else if (hf_amp > 0x64) {
		throw std::runtime_error("amplitude must be between 0x00 and 0x64.");
	}

	uint8_t amp_encoded = hf_amp;
	double amplitude;

	// maps to 0 - 1.00295
	if (amp_encoded == 0x00) {
		amplitude = 0.0;
	}
	else if (amp_encoded <= 0x0F) {
		amplitude = 18.0 / 5.0*std::pow(2.0, static_cast<double>(amp_encoded + 1) / 4.0 - 9.0);
	}
	else if (amp_encoded <= 0x1F) {
		amplitude = 18.0 / 5.0*std::pow(2.0, static_cast<double>(amp_encoded + 1) / 16.0 - 6.0);
	}
	else {
		amplitude = 18.0 / 5.0*std::pow(2.0, static_cast<double>(amp_encoded + 1) / 32.0 - 5.0);
	}

	return std::max(amplitude, 1.0);
}
