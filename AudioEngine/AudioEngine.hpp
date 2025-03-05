#pragma once

#include "PrimitiveTypes.hpp"

#include "Vec.hpp"

#include "SoundInfo.hpp"

#include <string>
#include <optional>

#ifdef AUDIOENGINE_EXPORTS
#define AUDIOENGINE_API __declspec(dllexport)
#else
#define AUDIOENGINE_API __declspec(dllimport)
#endif

// based on https://www.youtube.com/watch?v=Vjm--AqG04Y
// debated whether to maintain the pointer-to-implementation style design presented in the talk.
// I think i will because a different implementation, need only supply the needed functions of AudioEngine

// i now see why the pointer to implementation was done. The DLL interface barrier is interesting.
// if i define a bunch of STL types as exported class members, then sizes and implementations might change
// so instead, i can just export a class with a bunch of public accessing functions that interact
// with a un-exported implementation. kinda clever

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
		auto loadAndPlaySound(const std::string& path, const std::string& soundName, const Vec3<f32>& pos = Vec3<f32>{ 0, 0, 0 }, f32 volumedB = 0) -> i32;
		auto stopChannel(i32 channelId) -> void;
		auto stopAllChannels() -> void;
		auto setChannel3dPosition(i32 channelId, const Vec3<f32>& pos) -> void;
		auto setChannelVolume(i32 channelId, f32 volumedB) -> void;
		auto isPlaying(i32 channelId) const -> bool;
		auto getPlayingSound(i32 channelId) const -> std::optional<SoundInfo>;
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
