#pragma once

#include "Song.hpp"

#include "json.hpp"

#include <AudioEngine.hpp>

#include <filesystem>
#include <array>
#include <iostream>
#include <fstream>
#include <format>
#include <random>
#include <algorithm>
#include <chrono>

namespace PersonalMusicPlayer {
	constexpr const auto validFormats = std::array{
		".aif", ".aiff",
		".asf", ".wma", ".wmv", // windows only
		".dls",
		".flac",
		".fsb", // limited encodings: PCM16, FADPCM, Vorbis, AT9, XMA, Opus.
		// additional if xbox, playstation, xbox series X/S, PS5, switch
		".it",
		".mid",
		".mod",
		".mp2", ".mp3", // also .wav's RIFF encoding
		".ogg",
		".asx", ".pls", ".m3u", ".wax", // playlist tags
		".raw",
		".s3m",
		".wav", // limited encodings: PCM, IMA ADPCM. ACM only on windows
		".xm",
		".mp4", ".m4a"
		// ios/tvOS: AAC, ALAC, MP3
		// UWP
		// Android
	};

	auto validExtension(const std::filesystem::path& path) -> bool { // https://fmod.com/docs/2.02/api/core-api-sound.html#fmod_sound_type
		auto extension = path.extension().string();
		//return std::ranges::contains(validFormats, extension); // :( no c++23 yet
		return std::ranges::any_of(validFormats, [&extension](const std::string& str) { return str == extension; });
	};

	auto loadSongFromPath(
		Audio::AudioEngine& engine,
		const Song& song,
		u32* loaded = nullptr, // optional. if provided, can be used for stats over over many invocations
		u32* alreadyLoaded = nullptr,
		u32* failedToLoad = nullptr
	) -> i8 {
		i8 statusCode = engine.loadSound(song.path, song.name);
		switch (statusCode) {
			case -1: 
				std::cerr << std::format(
					"Failed to load song: {}, with path {}\n",
					song.name,
					song.path.substr(0, song.path.size() - song.name.size())
				);
				if (failedToLoad)
					(*failedToLoad) += 1;
				break;
			case 0:
				std::cerr << std::format(
					"Possible duplicate song detected while loading: {}, with path {}. Aborting load.\n",
					song.name,
					song.path.substr(0, song.path.size() - song.name.size())
				);
				if (alreadyLoaded)
					(*alreadyLoaded) += 1;
				break;
			case 1:
				if (loaded)
					(*loaded) += 1;
				break;
		}
		return statusCode;
	};

	auto loadSongFromPath(
		Audio::AudioEngine& engine,
		const Song& song
	) -> i8 {
		return loadSongFromPath(engine, song, nullptr, nullptr, nullptr);
	}

	/*
	reads from config.json
	result should be vec<Song> only containing existing files (at call time) that also have valid extensions
	*/
	auto getSongsFromConfigFile() -> std::vector<Song> {
		std::ifstream f("config.json");
		nlohmann::json config = nlohmann::json::parse(f);

		std::vector<std::string> folders = config["musicLibrary"]["folders"];
		std::vector<std::string> recusiveFolders = config["musicLibrary"]["recusiveFolders"];
		std::vector<std::string> individualSongs = config["musicLibrary"]["individualFiles"];

		std::vector<Song> loadedSongs;

		u32 invalidPath = 0, wrongFileExtension = 0;

		for (const auto& folder : folders) {
			for (const auto& file : std::filesystem::directory_iterator(folder)) {
				const auto path = file.path();
				if (!file.exists()) // if invalid, invalid case
					invalidPath++;
				else if (!validExtension(path)) // else if invalid, invalid case
					wrongFileExtension++;
				else // otherwise success case. kinda cool structure
					loadedSongs.emplace_back(path.string(), path.stem().string());
			}
		}
		for (const auto& folder : recusiveFolders) {
			for (const auto& file : std::filesystem::recursive_directory_iterator(folder)) {
				const auto path = file.path();
				if (!file.exists())
					invalidPath++;
				else if (!validExtension(path))
					wrongFileExtension++;
				else
					loadedSongs.emplace_back(path.string(), path.stem().string());
			}
		}
		for (const std::string& filePath : individualSongs) {
			const auto path = std::filesystem::path(filePath);
			const auto file = std::filesystem::directory_entry(path);
			if (!file.exists())
				invalidPath++;
			else if (!validExtension(path))
				wrongFileExtension++;
			else
				loadedSongs.emplace_back(path.string(), path.stem().string());
		}

		std::cout << std::format(
			"Songs loaded: {}, invalid paths: {}, invalid file extension: {}\n",
			loadedSongs.size(), invalidPath, wrongFileExtension
		);

		return loadedSongs;
	}

	auto loadEntireLibrary(Audio::AudioEngine& engine) -> std::vector<Song> {
		auto songs = getSongsFromConfigFile();

		u32 loaded = 0, alreadyLoaded = 0, failedToLoad = 0;

		for (auto i = 0; i < songs.size(); i++) {
			if (loadSongFromPath(engine, songs[i], &loaded, &alreadyLoaded, &failedToLoad) <= 0) {
				songs.erase(songs.begin() + i); // on fail to load (invalid for whatever reason)
				i--;
			}
		}

		std::cout << std::format(
			"Songs loaded: {}, duplicates: {}, failed loads: {}, library size: {}\n",
			loaded, alreadyLoaded, failedToLoad, songs.size()
		);

		return songs;
	};

	auto shuffleSongs(std::vector<Song>& songs) -> void {
		std::random_device rd;
		std::mt19937 gen(rd());
		return std::shuffle(songs.begin(), songs.end(), gen);
	}

	// returns number of lines printed
	auto printPlayingSongInfo(Audio::AudioEngine& engine, i32 channelId) -> i32 {
		auto soundInfo = engine.getPlayingSound(channelId);
		if (soundInfo.has_value()) {
			std::cout << std::format(
				"Song: {}\n\t{}:{}\\{}:{}\n",
				soundInfo.value().getName(),
				std::chrono::duration_cast<std::chrono::minutes>(soundInfo.value().getDurationPlayed()).count(),
				std::chrono::duration_cast<std::chrono::seconds>(soundInfo.value().getDurationPlayed()).count() % 60,
				std::chrono::duration_cast<std::chrono::minutes>(soundInfo.value().getDuration()).count(),
				std::chrono::duration_cast<std::chrono::seconds>(soundInfo.value().getDuration()).count() % 60
			);
			return 2;
		}
		else {
			std::cout << std::format("No song info\n");
			return 1;
		}
	}

	auto printLibraryPositionInfo(const std::vector<Song>& library, i32 currentSongIndex) -> int {
		std::cout << std::format("Song {}\\{}\n", currentSongIndex + 1, library.size());
		return 1;
	}
};
