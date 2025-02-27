
#include "pch.h"

#include "SoundInfoImpl.hpp"


constexpr const static auto stringEndTrim = [](std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
};

SoundInfoImpl::SoundInfoImpl(FMOD::Sound* sound, FMOD::Channel* channel) : tags{} {
	constexpr const static auto nameBufferLength = 100;
	// get name
	char* nameBuffer = new char[nameBufferLength];
	for (auto i = 0; i < nameBufferLength; i++) {
		nameBuffer[i] = ' ';
	}
	sound->getName(nameBuffer, nameBufferLength);
	this->name = std::string(nameBuffer);
	stringEndTrim(this->name);
	delete[] nameBuffer;

	// get format
	FMOD_SOUND_TYPE typeEnum;
	FMOD_SOUND_FORMAT formatEnum;
	sound->getFormat(&typeEnum, &formatEnum, &this->channels, &this->bitsPerSample);
	this->setFormat(formatEnum);
	this->setType(typeEnum);

	// get length
	u32 soundLength;
	sound->getLength(&soundLength, FMOD_TIMEUNIT_MS);
	this->duration = std::chrono::milliseconds(soundLength);

	// get current playback length
	if (channel) {
		u32 position;
		channel->getPosition(&position, FMOD_TIMEUNIT_MS);
		this->durationPlayed = std::chrono::milliseconds(position);
	}
	else
		this->durationPlayed = std::chrono::milliseconds::zero();

	// get amount of tags
	i32 numTags, numTagsUpdated;
	sound->getNumTags(&numTags, &numTagsUpdated);

	// get tags
	FMOD_TAG tag;
	for (auto i = 0; i < numTags; i++) {
		sound->getTag(nullptr, i, &tag);
		this->setTagData(tag);
	}
}

auto SoundInfoImpl::setFormat(FMOD_SOUND_FORMAT f) -> void {
	switch (f) {
		case FMOD_SOUND_FORMAT_PCM8:
			this->format = "pcm8";
			break;
		case FMOD_SOUND_FORMAT_PCM16:
			this->format = "pcm16";
			break;
		case FMOD_SOUND_FORMAT_PCM24:
			this->format = "pcm24";
			break;
		case FMOD_SOUND_FORMAT_PCM32:
			this->format = "pcm32";
			break;
		case FMOD_SOUND_FORMAT_PCMFLOAT:
			this->format = "pcm_float";
			break;
		case FMOD_SOUND_FORMAT_BITSTREAM:
			this->format = "bitstream";
			break;
		case FMOD_SOUND_FORMAT_MAX:
			this->format = "max";
			break;
		case FMOD_SOUND_FORMAT_NONE:
		default:
			this->format = "unknown";
			break;
	}
}

auto SoundInfoImpl::setType(FMOD_SOUND_TYPE t) -> void {
	switch (t) {
		case FMOD_SOUND_TYPE_AIFF:
			this->type = "aiff";
			break;
		case FMOD_SOUND_TYPE_ASF:
			this->type = "asf";
			break;
		case FMOD_SOUND_TYPE_DLS:
			this->type = "dls";
			break;
		case FMOD_SOUND_TYPE_FLAC:
			this->type = "flac";
			break;
		case FMOD_SOUND_TYPE_FSB:
			this->type = "fsb";
			break;
		case FMOD_SOUND_TYPE_IT:
			this->type = "it";
			break;
		case FMOD_SOUND_TYPE_MIDI:
			this->type = "mid";
			break;
		case FMOD_SOUND_TYPE_MOD:
			this->type = "mod";
			break;
		case FMOD_SOUND_TYPE_MPEG:
			this->type = "mpeg";
			break;
		case FMOD_SOUND_TYPE_OGGVORBIS:
			this->type = "ogg";
			break;
		case FMOD_SOUND_TYPE_PLAYLIST:
			this->type = "playlist";
			break;
		case FMOD_SOUND_TYPE_RAW:
			this->type = "raw";
			break;
		case FMOD_SOUND_TYPE_S3M:
			this->type = "s3m";
			break;
		case FMOD_SOUND_TYPE_USER:
			this->type = "user";
			break;
		case FMOD_SOUND_TYPE_WAV:
			this->type = "wav";
			break;
		case FMOD_SOUND_TYPE_XM:
			this->type = "xm";
			break;
		case FMOD_SOUND_TYPE_XMA:
			this->type = "xma";
			break;
		case FMOD_SOUND_TYPE_AUDIOQUEUE:
			this->type = "audioqueue";
			break;
		case FMOD_SOUND_TYPE_AT9:
			this->type = "at9";
			break;
		case FMOD_SOUND_TYPE_VORBIS:
			this->type = "vorbis";
			break;
		case FMOD_SOUND_TYPE_MEDIA_FOUNDATION:
			this->type = "mediafoundation";
			break;
		case FMOD_SOUND_TYPE_MEDIACODEC:
			this->type = "mediacodec";
			break;
		case FMOD_SOUND_TYPE_FADPCM:
			this->type = "fadpcm";
			break;
		case FMOD_SOUND_TYPE_OPUS:
			this->type = "opus";
			break;
		case FMOD_SOUND_TYPE_MAX:
			this->type = "max";
			break;
		case FMOD_SOUND_TYPE_UNKNOWN:
		default:
			this->type = "unknown";
			break;
	}
}

auto SoundInfoImpl::setTagData(FMOD_TAG t) -> void {
	std::string tagName(t.name);
	std::string tagData;
	switch (t.datatype) {
		case FMOD_TAGDATATYPE_STRING:
			[[fallthrough]];
		case FMOD_TAGDATATYPE_STRING_UTF8:
			[[fallthrough]];
		case FMOD_TAGDATATYPE_STRING_UTF16:
			[[fallthrough]];
		case FMOD_TAGDATATYPE_STRING_UTF16BE:
			[[fallthrough]];
		case FMOD_TAGDATATYPE_MAX:
			[[fallthrough]];
		case FMOD_TAGDATATYPE_BINARY:
			tagData = std::string(static_cast<char*>(t.data));
			break;
		case FMOD_TAGDATATYPE_INT: // could be 8-64 bits
			switch (t.datalen) {
				case 1: // 8 bit int
					tagData = std::to_string(*static_cast<i8*>(t.data));
					break;
				case 2: // 16 bit int
					tagData = std::to_string(*static_cast<i16*>(t.data));
					break;
				case 4: // 32 bit int
					tagData = std::to_string(*static_cast<i32*>(t.data));
					break;
				case 8: // 64 bit int
					tagData = std::to_string(*static_cast<i64*>(t.data));
					break;
				default:
					// error
					break;
			}
			break;
		case FMOD_TAGDATATYPE_FLOAT:
			switch (t.datalen) {
				case 4: // 32 bit float
					tagData = std::to_string(*static_cast<f32*>(t.data));
					break;
				case 8: // 64 bit float
					tagData = std::to_string(*static_cast<f64*>(t.data));
					break;
				default:
					// error
					break;
			}
	}
	this->tags[tagName] = tagData;
}
