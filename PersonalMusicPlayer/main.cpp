#include "PrimitiveTypes.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <algorithm>

#include "json.hpp"

#include "AudioEngine.hpp"

#include "Song.hpp"

using json = nlohmann::json;

auto main() -> int {
	const auto locale = "en_US.UTF-8";
	std::setlocale(LC_ALL, locale);
	std::locale::global(std::locale(locale)); // need locales for dealing with string conversions (maybe)

	Audio::AudioEngine::init();

	Audio::AudioEngine engine{};

	std::ifstream f("config.json");
	json config = json::parse(f);

	std::vector<std::string> folders = config["musicLibrary"]["folders"];
	std::vector<std::string> recusiveFolders = config["musicLibrary"]["recusiveFolders"];
	std::vector<std::string> individualSongs = config["musicLibrary"]["individualFiles"];

	std::vector<Song> loadedSongs;

	i32 failedToLoad = 0, alreadyLoaded = 0, loaded = 0;
	auto loadSongFromPath = [&engine, &failedToLoad, &alreadyLoaded, &loaded](const std::filesystem::path& path, std::vector<Song>& loadedSongs) {
		std::string pathAsString = path.string();
		Song& curr = loadedSongs.emplace_back(pathAsString, path.stem().string());
		i8 statusCode = engine.loadSound(curr.path, curr.name);
		switch (statusCode) {
			case -1: 
				std::cerr << std::format(
					"Failed to load song: {}, with path {}\n",
					curr.name,
					curr.path.substr(0, curr.path.size() - curr.name.size())
				);
				failedToLoad++;
				loadedSongs.pop_back();
				break;
			case 0:
				std::cerr << std::format(
					"Possible duplicate song detected while loading: {}, with path {}. Aborting load.\n",
					curr.name,
					curr.path.substr(0, curr.path.size() - curr.name.size())
				);
				alreadyLoaded++;
				loadedSongs.pop_back();
				break;
			case 1:
				loaded++;
				break;
		}
	};

	auto validExtension = [](const std::filesystem::path& path) -> bool {
		constexpr static auto validFormats = std::array{
			".aif", ".aiff",
			".asf", ".wma", ".wmv",
			".dls",
			".flac",
			".fsb",
			".it",
			".mid",
			".mod",
			".mp2", ".mp3",
			".ogg",
			".asx", ".pls", ".m3u", ".wax",
			".raw",
			".s3m",
			".wav",
			".xm",
			".mp4", ".m4a"
		};
		auto extension = path.extension().string();
		//return std::ranges::contains(validFormats, extension); // :( no c++23 yet
		return std::ranges::any_of(validFormats, [&extension](const std::string& str) { return str == extension; });
	};

	i32 wrongFileExtension = 0;
	for (const auto& folder : folders) {
		for (const auto& file : std::filesystem::directory_iterator(folder)) {
			const auto path = file.path().string();
			if (validExtension(path))
				loadSongFromPath(path, loadedSongs);
			else
				wrongFileExtension++;
		}
	}
	for (const auto& folder : recusiveFolders) {
		for (const auto& file : std::filesystem::recursive_directory_iterator(folder)) {
			const auto path = file.path().string();
			if (validExtension(path))
				loadSongFromPath(path, loadedSongs);
			else
				wrongFileExtension++;
		}
	}
	for (const std::string& filePath : individualSongs) {
		const auto path = std::filesystem::path(filePath);
		if (validExtension(path))
			loadSongFromPath(path, loadedSongs);
		else
			wrongFileExtension++;
	}
	
	std::cout << std::format(
		"Songs loaded: {}, duplicates: {}, failed loads: {}, invalid file extension: {}, library size: {}\n",
		loaded, alreadyLoaded, failedToLoad, wrongFileExtension, loadedSongs.size()
	);
	

	i32 channelId = engine.playSound(loadedSongs[0].name);
	//R"(H:\projects\js\ytPlaylistDownloader\out\Answer to Extreme TOGENASHI TOGEARI - Topic.mp3)"

	while (engine.isPlaying(channelId)) {
		engine.update();
	}

	std::cout << " sound over" << std::endl;

	return 0;
}