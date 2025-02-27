#pragma once

#include "PrimitiveTypes.hpp"

#include <chrono>
#include <memory>
#include <unordered_map>

#ifdef AUDIOENGINE_EXPORTS
#define AUDIOENGINE_API __declspec(dllexport)
#else
#define AUDIOENGINE_API __declspec(dllimport)
#endif

namespace Audio {
	// i intend to return this class to the user. so i dont need to let them construct them.
	// so no need to export the constructor? Makes sense for now, let's see how that goes.
	class SoundInfo {
	public:
		SoundInfo(void* sound, void* channel = nullptr);
		AUDIOENGINE_API ~SoundInfo(); // i think i need to export this so the deconstructor gets called
		AUDIOENGINE_API auto getName() -> const std::string&;
		AUDIOENGINE_API auto getFormat() -> const std::string&;
		AUDIOENGINE_API auto getDuration() -> std::chrono::milliseconds;
		AUDIOENGINE_API auto getDurationPlayed() -> std::chrono::milliseconds;
		AUDIOENGINE_API auto getTags() -> const std::unordered_map<std::string, std::string>&;
	private:
		void* impl; // one per soundInfo, but not exported
	};
	// huh, the void pointer thing actually seems to work. kinda nasty. prob a better way to do this
};
