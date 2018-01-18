#include <stdexcept>

#include "homelight.h"
#include "types.h"

void HOME_LIGHT::set_header(unsigned char amount_mini, unsigned char global_duration, unsigned char start_intensity, unsigned char amount_full) {
	vec[0] = (amount_mini << 4) + (global_duration & 0xF);
	vec[1] = (start_intensity << 4) + (amount_full & 0xF);
}

// deletes all higher nibbles
void HOME_LIGHT::set_mini_cycle(unsigned char idx, unsigned char intensity, unsigned char fading_transition_duration, unsigned char duration_multiplier) {

	idx &= 0xF;

	if (idx == 0) {
		throw std::invalid_argument("idx must be greater than 0.");
	}

	unsigned char bit_offset, data_byte, intensity_byte;
	if (idx % 2 == 1) {
		bit_offset = 4;
		intensity_byte = 2 + 3 * (idx - 1) / 2;
		data_byte = intensity_byte + 1;
	}
	else {
		bit_offset = 0;
		intensity_byte = 2 + 3 * (idx - 2) / 2;
		data_byte = intensity_byte + 2;
	}
	vec[intensity_byte] = (intensity & 0xF) << bit_offset;
	vec[data_byte] = (fading_transition_duration << 4) + (duration_multiplier & 0xF);
}