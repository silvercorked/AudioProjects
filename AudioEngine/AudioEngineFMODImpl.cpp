
#include "pch.h"

#include "AudioEngineFMODImpl.hpp"

#include <vector>
#include <cassert>

AudioEngineFMODImpl::AudioEngineFMODImpl() : nextChannelId(1) {
	assert(FMOD::System_Create(&this->system) == FMOD_OK);
	assert(this->system->init(32, FMOD_INIT_NORMAL, nullptr) == FMOD_OK);
	assert(this->system->createChannelGroup("main", &this->channelGroup) == FMOD_OK);

	this->system->set3DNumListeners(1);
}

AudioEngineFMODImpl::~AudioEngineFMODImpl() {
	for (auto& channel : this->channels) {
		channel.second->stop();
	}
	this->channels.clear(); // these seem to not need to be released. I think the channels might just be ids for internal
	// structures inside the system, so i think the system release handles it
	for (auto& sound : this->sounds) {
		sound.second->release();
	}
	this->sounds.clear();
	this->channelGroup->release();
	this->system->release();
}

auto AudioEngineFMODImpl::update() -> void {
	std::vector<ChannelMap::iterator> stoppedChannels;
	for (auto iter = this->channels.begin(), iterEnd = this->channels.end(); iter != iterEnd; iter++) {
		bool isPlaying = false;
		iter->second->isPlaying(&isPlaying);
		if (!isPlaying) {
			stoppedChannels.push_back(iter);
		}
	}
	for (auto& channel : stoppedChannels) {
		this->channels.erase(channel);
	}
	this->system->update();
}
