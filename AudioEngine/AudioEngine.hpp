#pragma once

#include "PrimitiveTypes.hpp"

#include "Vec.hpp"

#include <string>

#ifdef AUDIOENGINE_EXPORTS
#define AUDIOENGINE_API __declspec(dllexport)
#else
#define AUDIOENGINE_API __declspec(dllimport)
#endif

// based on https://www.youtube.com/watch?v=Vjm--AqG04Y
// debated whether to maintain the pointer-to-implementation style design presented in the talk.
// I think i will because a different implementation, need only supply the needed functions of AudioEngine

namespace Audio {
	class AUDIOENGINE_API AudioEngine {
	public:
		static auto init() -> void;
		static auto update() -> void;
		static auto shutdown() -> void;

		auto loadSound(const std::string& soundName, bool space3d = true, bool looping = false, bool stream = false) -> void;
		auto loadSound(const std::string& path, const std::string& soundName, bool space3d = true, bool looping = false, bool stream = false) -> int;
		auto unloadSound(const std::string& soundName) -> void;
		auto set3dListenerAndOrientation(const Vec3<f32>& pos, const Vec3<f32>& look, const Vec3<f32>& up) -> void;
		auto playSound(const std::string& soundName, const Vec3<f32>& pos = Vec3<f32>{ 0, 0, 0 }, f32 volumedB = 0) -> i32;
		auto stopChannel(i32 channelId) -> void;
		auto stopAllChannels() -> void;
		auto setChannel3dPosition(i32 channelId, const Vec3<f32>& pos) -> void;
		auto setChannelVolume(i32 channelId, f32 volumedB) -> void;
		auto isPlaying(i32 channelId) const -> bool;
	};
};



/*
	Things to add (from the talk):
	Streaming
	Compressed Sample Loading
	Designer Tools Integration
	Localization
	Memory Management
	Internal Multithreading
	Time-synching
	Asynchronous Loads
	Mixing Tools
	Music
		Dynamic Music
	Background Sounds/Ambience/Environment
	Reverb
	DSP Effects
	Platform Specific Requirements
	Cross-Platform Initialization
	Surrounding Panning and Multichannel Sounds
	Custom Listener Manipulations
	Obstruction/Occlusion
	Asset Packaging
	Audio Compression Formats
*/
