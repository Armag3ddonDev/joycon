#include <cmath>

#include "buffer.h"
#include "rumble.h"

/*
void Rumble::pack() {

	bool offset = false;
	if (type == JOYCON::LEFT) {
		offset = false;
	}
	else if (type == JOYCON::RIGHT) {
		offset = true;
	}
	else {
		throw std::invalid_argument("Invalid JOYCON type.");
	}

	uint16_t hf;
	uint8_t lf;
	uint8_t hf_amp;
	uint16_t lf_amp;

	encode_frequency(frequency, hf, lf);
	encode_amplitude(amplitude, hf_amp, lf_amp);

	//Byte swapping
	buf[2 + offset * 4] = hf & 0xFF;
	buf[3 + offset * 4] = hf_amp + ((hf >> 8) & 0xFF); //Add amp + 1st byte of frequency to amplitude byte
													   //Byte swapping
	buf[4 + offset * 4] = lf + ((lf_amp >> 8) & 0xFF); //Add freq + 1st byte of LF amplitude to the frequency byte
	buf[5 + offset * 4] = lf_amp & 0xFF;
}
*/

void Rumble::unpack() {

}

void Rumble::encode_frequency(double frequency, uint16_t& hf, uint8_t& lf) const {

	// Float frequency to hex conversion
	if (frequency < 40.87) {
		frequency = 40.87;
	}
	else if (frequency > 1252.57) {
		frequency = 1252.57;
	}

	// maps to 0x41(65) - 0xDF(223)
	uint8_t encoded_hex_freq = static_cast<uint8_t>(round(log2(frequency / 10.0)*32.0));

	// Convert to Joy-Con HF range. Range in big-endian: 0x0004(81.75Hz)-0x01FC(1252.57Hz) with +0x0004 steps.
	hf = (encoded_hex_freq - 0x60) * 4;	// what happens under 81.75Hz?
										// Convert to Joy-Con LF range. Range: 0x01-0x7F.
	lf = encoded_hex_freq - 0x40;
}

void Rumble::encode_amplitude(double amplitude, uint8_t& hf_amp, uint16_t& lf_amp) const {

	// dont increase for safety-reasons of the linear resonant actuators. max safe is 1.002867
	if ((amplitude < 0.0) || (amplitude > 1.0)) {
		throw std::invalid_argument("amplitude must be between 0 and 1.");
	}

	// maps to 0x00(0) - 0xC8(100)
	uint8_t amp_encoded;
	if (amplitude < 0.008) {
		amp_encoded = 0x00;
	}
	else if (amplitude < 0.112491) {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 9.0)*4.0)) - 1;
	}
	else if (amplitude < 0.224982) {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 6.0)*16.0)) - 1;
	}
	else {
		amp_encoded = static_cast<uint8_t>(round((log2(amplitude * 5.0 / 18.0) + 5.0)*32.0)) - 1;
	}

	// saveguard
	if (amp_encoded > 0xC8) {
		throw std::bad_exception();
	}

	hf_amp = amp_encoded << 1;	// multiply by 2
	lf_amp = 0x8000 * (amp_encoded & 0x01) + 0x40 + (amp_encoded >> 1);	// 0x80XX if odd, else 0x00XX
}

double Rumble::decode_frequency(uint16_t hf, uint8_t lf) const {

	uint8_t encoded_hex_freq = lf + 0x40;
	if (encoded_hex_freq != ((hf >> 2) + 0x60)) {
		throw std::runtime_error("lf (" + std::to_string(lf) + ") and hf (" + std::to_string(hf) + ") produce different frequencies!");;
	}

	return 10.0*std::pow(2.0, static_cast<float>(encoded_hex_freq) / 32.0);
}

double Rumble::decode_amplitude(uint8_t hf_amp, uint16_t lf_amp) const {

	uint8_t amp_encoded = hf_amp >> 1;
	if (lf_amp != (0x8000 * (amp_encoded & 0x01) + 0x40 + (amp_encoded >> 1))) {
		throw std::runtime_error("lf_amp (" + std::to_string(lf_amp) + ") and hf_amp (" + std::to_string(hf_amp) + ") produce different frequencies!");;
	}

	// saveguard
	if (amp_encoded > 0xC8) {
		throw std::bad_exception();
	}

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

	// amplitude of 0xC8 is 1.002948 because of 'round' during encoding
	if ((amplitude < 0.0) || (amplitude > 1.002948)) {
		throw std::runtime_error("Something went wrong. Amplitude should be between 0 and 1.");
	}
	return std::max(amplitude, 1.0);
}
