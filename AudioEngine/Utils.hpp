#pragma once

#include "PrimitiveTypes.hpp"

#ifdef AUDIOENGINE_EXPORTS
#define AUDIOENGINE_API __declspec(dllexport)
#else
#define AUDIOENGINE_API __declspec(dllimport)
#endif

namespace Audio {
	AUDIOENGINE_API
	auto dBToVolume(f32 dB) -> f32;

	AUDIOENGINE_API
	auto volumeTodB(f32 volume) -> f32;
};
