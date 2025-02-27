#pragma once

#include "PrimitiveTypes.hpp"

#include "fmod.hpp"

#include <string>
#include <chrono>
#include <unordered_map>

struct SoundInfoImpl {
	std::string name;
	std::string format; // could make these enums but dont really need them right now for anything
	std::string type; // besides info and would have to export another type
	i32 channels;
	i32 bitsPerSample;
	std::chrono::milliseconds duration;
	std::chrono::milliseconds durationPlayed;
	std::unordered_map<std::string, std::string> tags;

	SoundInfoImpl(FMOD::Sound* sound, FMOD::Channel* channel = nullptr);

private:
	auto setFormat(FMOD_SOUND_FORMAT f) -> void;
	auto setType(FMOD_SOUND_TYPE t) -> void;
	auto setTagData(FMOD_TAG t) -> void;
};
