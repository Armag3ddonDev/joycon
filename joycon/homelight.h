#pragma once

#include "types.h"

// *WARNING* : All inputs are only nibbles (4bit). Everything else gets cut off.
class HOME_LIGHT {
public:
	HOME_LIGHT() : vec(25, 0) {}

	// HEADER
	// Byte 0 (HIGH) - amount_mini		: 0 - 15
	// Byte 0  (LOW) - global_duration	: 8ms - 175ms, x0 = 0ms(OFF)
	// Byte 1 (HIGH) - start_intensity	: x0 = 0%, xF = 100%
	// Byte 1 ( LOW) - amount_full		: 1 - 15, x0 = repeat forever,
	void set_header(unsigned char amount_mini, unsigned char global_duration, unsigned char start_intensity, unsigned char amount_full);

	// deletes all higher nibbles
	void set_mini_cycle(unsigned char idx, unsigned char intensity, unsigned char fading_transition_duration, unsigned char duration_multiplier);

	const ByteVector& data() const { return vec; }

	ByteVector vec;
};
