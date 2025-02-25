
#include "pch.h"

#include "Utils.hpp"

#include <cmath>

namespace Audio {
	auto dBToVolume(f32 dB) -> f32 {
		return std::powf(10.0f, 0.05f * dB);
	}

	auto volumeTodB(f32 volume) -> f32 {
		return 20.0f * std::log10f(volume);
	}
};