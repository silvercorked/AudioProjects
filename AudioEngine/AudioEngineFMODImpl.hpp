#pragma once

#include "PrimitiveTypes.hpp"

#include "fmod.hpp"

#include <map>
#include <string>

struct AudioEngineFMODImpl {
	typedef std::map<std::string, FMOD::Sound*> SoundMap;
	typedef std::map<i32, FMOD::Channel*> ChannelMap;

	AudioEngineFMODImpl();
	~AudioEngineFMODImpl();

	auto update() -> void;

	FMOD::System* system;
	i32 nextChannelId;
	FMOD::ChannelGroup* channelGroup;
	SoundMap sounds;
	ChannelMap channels;
};