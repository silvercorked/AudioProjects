
#include "pch.h"

#include "AudioEngine.hpp"

#include "AudioEngineFMODImpl.hpp"

namespace Audio {
	AudioEngineFMODImpl* impl = nullptr;
	
	auto Vec3ToFMODVec(const Vec3<f32>& in) -> FMOD_VECTOR {
		return FMOD_VECTOR{ in.x, in.y, in.z };
	}

	auto AudioEngine::init() -> void {
		impl = new AudioEngineFMODImpl();
	}

	auto AudioEngine::update() -> void {
		impl->update();
	}

	auto AudioEngine::shutdown() -> void {
		delete impl;
	}

	auto AudioEngine::loadSound(const std::string& path, const std::string& soundName, bool space3d, bool looping, bool stream) -> int {
		auto foundIter = impl->sounds.find(soundName);
		if (foundIter != impl->sounds.end()) return 0; // sound by that name already exists

		FMOD_MODE mode = FMOD_DEFAULT;
		mode |= space3d ? FMOD_3D : FMOD_2D;
		mode |= looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;
		FMOD::Sound* sound = nullptr;
		impl->system->createSound(path.c_str(), mode, nullptr, &sound);
		if (sound) {
			impl->sounds[soundName] = sound;
			return 1; // success in creating new sound
		}
		return -1; // failed to create new sound
	}

	auto AudioEngine::loadSound(const std::string& soundName, bool space3d, bool looping, bool stream) -> void {
		this->loadSound(soundName, soundName, space3d, looping, stream);
	}

	auto AudioEngine::unloadSound(const std::string& soundName) -> void {
		auto foundIter = impl->sounds.find(soundName);
		if (foundIter == impl->sounds.end()) return;
		foundIter->second->release();
		impl->sounds.erase(foundIter);
	}

	auto AudioEngine::set3dListenerAndOrientation(const Vec3<f32>& pos, const Vec3<f32>& look, const Vec3<f32>& up) -> void {
		FMOD_VECTOR position = Vec3ToFMODVec(pos);
		FMOD_VECTOR looking = Vec3ToFMODVec(look);
		FMOD_VECTOR upward = Vec3ToFMODVec(up);
		impl->system->set3DListenerAttributes(0, &position, nullptr, &looking, &upward);
	}

	auto AudioEngine::playSound(const std::string& soundName, const Vec3<f32>& pos, f32 volumedB) -> i32 {
		i32 channelId = impl->nextChannelId++;
		auto foundIter = impl->sounds.find(soundName);
		if (foundIter == impl->sounds.end()) {
			this->loadSound(soundName);
			foundIter = impl->sounds.find(soundName);
			if (foundIter == impl->sounds.end()) {
				return channelId; // this is a failure case, but return valid channel id anyway
			}
		}
		FMOD::Channel* channel = nullptr;
		impl->system->playSound(foundIter->second, impl->channelGroup, true, &channel);
		if (channel) { // don't want to play sound automatically because still need to set some values on the channel
			FMOD_VECTOR position = Vec3ToFMODVec(pos);
			channel->set3DAttributes(&position, nullptr);
			channel->setVolume(dBToVolume(volumedB));
			channel->setPaused(false);
			impl->channels[channelId] = channel;
		}
		return channelId;
	}

	auto AudioEngine::loadAndPlaySound(const std::string& path, const std::string& soundName, const Vec3<f32>& pos, f32 volumedB) -> i32 {
		i32 channelId = impl->nextChannelId++;
		if (this->loadSound(path, soundName) < 0) {
			return channelId; // failed to load
		}
		auto foundIter = impl->sounds.find(soundName);
		if (foundIter == impl->sounds.end()) {
			return channelId; // error case. shouldn't happen unless multiple threads involved
		}
		
		FMOD::Channel* channel = nullptr;
		impl->system->playSound(foundIter->second, impl->channelGroup, true, &channel);
		if (channel) { // don't want to play sound automatically because still need to set some values on the channel
			FMOD_VECTOR position = Vec3ToFMODVec(pos);
			channel->set3DAttributes(&position, nullptr);
			channel->setVolume(dBToVolume(volumedB));
			channel->setPaused(false);
			impl->channels[channelId] = channel;
		}
		return channelId;
	}

	auto AudioEngine::stopChannel(i32 channelId) -> void {
		auto foundIter = impl->channels.find(channelId);
		if (foundIter == impl->channels.end()) return;
		foundIter->second->stop();
	}

	auto AudioEngine::stopAllChannels() -> void {
		for (auto& channel : impl->channels) {
			channel.second->stop();
		}
	}

	auto AudioEngine::setChannel3dPosition(i32 channelId, const Vec3<f32>& pos) -> void {
		auto foundIter = impl->channels.find(channelId);
		if (foundIter == impl->channels.end()) return;
		FMOD_VECTOR position = Vec3ToFMODVec(pos);
		foundIter->second->set3DAttributes(&position, nullptr);
	}

	auto AudioEngine::setChannelVolume(i32 channelId, f32 volumedB) -> void {
		auto foundIter = impl->channels.find(channelId);
		if (foundIter == impl->channels.end()) return;
		foundIter->second->setVolume(dBToVolume(volumedB));
	}

	auto AudioEngine::isPlaying(i32 channelId) const -> bool {
		auto foundIter = impl->channels.find(channelId);
		if (foundIter == impl->channels.end())
			return false;
		bool playing;
		foundIter->second->isPlaying(&playing);
		return playing;
	}
	auto AudioEngine::getPlayingSound(i32 channelId) const -> std::optional<SoundInfo> {
		auto foundIter = impl->channels.find(channelId);
		if (foundIter != impl->channels.end()) {
			FMOD::Sound* sound;
			foundIter->second->getCurrentSound(&sound);
			if (sound) {
				return std::optional<SoundInfo>(std::in_place, sound, foundIter->second);
			}
		}
		 return std::optional<SoundInfo>{};
	}
};
