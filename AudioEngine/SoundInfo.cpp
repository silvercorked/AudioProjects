
#include "pch.h"

#include "SoundInfo.hpp"

#include "AudioEngine.hpp"

#include "SoundInfoImpl.hpp"

namespace Audio {
	/*
		not my favorite idea but i can't call dll dependencies (fmod.hpp) in hpp files that get exported
		also can't use the pch.h (as it contains fmod.hpp) in those hpp files. So i can pass void ptrs
		and reinterpret to proper types here. Otherwise i need access to AudioEngineFMODImpl to get references
		to the sound map and channel map.
	*/
	SoundInfo::SoundInfo(void* sound, void* channel) {
		FMOD::Sound* s = reinterpret_cast<FMOD::Sound*>(sound); // gotta pull out the big guns i think
		FMOD::Channel* c = nullptr;
		if (channel)
			c = reinterpret_cast<FMOD::Channel*>(channel);
		this->impl = new SoundInfoImpl(s, c);
	}

	SoundInfo::~SoundInfo() {
		delete (static_cast<SoundInfoImpl*>(this->impl));
	}

	auto SoundInfo::getName() -> const std::string& {
		return static_cast<SoundInfoImpl*>(this->impl)->name;
	}

	auto SoundInfo::getFormat() -> const std::string& {
		return static_cast<SoundInfoImpl*>(this->impl)->format;
	}

	auto SoundInfo::getDuration() -> std::chrono::milliseconds {
		return static_cast<SoundInfoImpl*>(this->impl)->duration;
	}

	auto SoundInfo::getDurationPlayed() -> std::chrono::milliseconds {
		return static_cast<SoundInfoImpl*>(this->impl)->durationPlayed;
	}

	auto SoundInfo::getTags() -> const std::unordered_map<std::string, std::string>& {
		return static_cast<SoundInfoImpl*>(this->impl)->tags;
	}
};


