
#include "PrimitiveTypes.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ranges>
#include <algorithm>

#include "json.hpp"

#include "AudioEngine.hpp"

#include "API.hpp"
#include "Song.hpp"
#include "LoadedSong.hpp"
#include "Input.hpp"

#include "TerminalUtils.hpp"

auto main() -> int {
	const auto locale = "en_US.UTF-8";
	std::setlocale(LC_ALL, locale);
	std::locale::global(std::locale(locale)); // need locales for dealing with string conversions (maybe)

	Audio::AudioEngine::init();
	
	Audio::AudioEngine engine{};

	auto& input = Input::getInstance();
	input.registerKeyToAction('N', KeyActions::nextSong);
	input.registerKeyToAction('B', KeyActions::prevSong);
	input.registerKeyToAction('P', KeyActions::togglePaused);
	input.registerKeyToAction('S', KeyActions::shuffleSongs);
	input.registerKeyToAction('Q', KeyActions::quitApplication);

	//auto songs = PersonalMusicPlayer::loadEntireLibrary(engine);
	// avoid preload
	auto songs = PersonalMusicPlayer::getSongsFromConfigFile();
	std::cout << "\n\n";
	LoadedSong playingSong;

	PersonalMusicPlayer::shuffleSongs(songs);

	i32 currentSongIndex = 0;
	bool quit = false;

	i32 channelId = engine.loadAndPlaySound(songs[currentSongIndex].path, songs[currentSongIndex].name);
	playingSong = LoadedSong(songs[currentSongIndex], channelId);

	std::mutex audioMutex;

	input.subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			engine.unloadSound(playingSong.song.name);
			currentSongIndex++;
			if (currentSongIndex >= songs.size())
				currentSongIndex = 0;
			i32 newChannelId = engine.loadAndPlaySound(songs[currentSongIndex].path, songs[currentSongIndex].name);
			playingSong = LoadedSong(songs[currentSongIndex], newChannelId);
		}, KeyActions::nextSong
	);
	input.subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			engine.unloadSound(playingSong.song.name);
			currentSongIndex--;
			if (currentSongIndex < 0)
				currentSongIndex = songs.size() - 1;
			i32 newChannelId = engine.loadAndPlaySound(songs[currentSongIndex].path, songs[currentSongIndex].name);
			playingSong = LoadedSong(songs[currentSongIndex], newChannelId);
		}, KeyActions::prevSong
	);
	input.subscribeToKeypress(
		[&quit]() -> void {
			quit = true;
		}, KeyActions::quitApplication
	);
	input.subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			PersonalMusicPlayer::shuffleSongs(songs);
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			engine.unloadSound(playingSong.song.name);
			i32 newChannelid = engine.loadAndPlaySound(songs[currentSongIndex].path, songs[currentSongIndex].name);
			playingSong = LoadedSong(songs[currentSongIndex], newChannelid);
		}, KeyActions::shuffleSongs
	);

	i32 linesUsed = PersonalMusicPlayer::printLibraryPositionInfo(songs, currentSongIndex);
	linesUsed += PersonalMusicPlayer::printPlayingSongInfo(engine, channelId);
	auto lastTimePoint = std::chrono::steady_clock::now();
	auto currTimePoint = lastTimePoint;
	
	while (!quit) {
		// print music menu for song selection
		while (engine.isPlaying(playingSong.channelId)) {
			engine.update();

			currTimePoint = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(currTimePoint - lastTimePoint).count() >= 1) {
				eraseLines(linesUsed);
				{
					std::lock_guard<std::mutex> lock(audioMutex);
					linesUsed = PersonalMusicPlayer::printLibraryPositionInfo(songs, currentSongIndex);
					linesUsed += PersonalMusicPlayer::printPlayingSongInfo(engine, playingSong.channelId);
				}
				lastTimePoint = currTimePoint;
			}
			{ // before trying cases, wait mutex (allows full nextSong/prevSong behavior before checking engine.isPlaying)
				std::lock_guard<std::mutex> lock(audioMutex);
				if (!quit && !engine.isPlaying(playingSong.channelId)) { // song ended naturally
					if (engine.isPlaying(playingSong.channelId))
						engine.stopChannel(playingSong.channelId);
					engine.unloadSound(playingSong.song.name);
					currentSongIndex++;
					if (currentSongIndex >= songs.size())
						currentSongIndex = 0;
					i32 newChannelid = engine.loadAndPlaySound(songs[currentSongIndex].path, songs[currentSongIndex].name);
					playingSong = LoadedSong(songs[currentSongIndex], newChannelid);
				}
				if (quit) {
					engine.stopAllChannels();
					engine.unloadSound(playingSong.song.name);
					break;
				}
			}
		}
	}

	/*
	if (keyboardActions.nextSong) {
		currentSongIndex++;
		if (currentSongIndex >= songs.size())
			currentSongIndex = 0;
		if (engine.isPlaying(channelId))
			engine.stopChannel(channelId);
		i32 newChannelid = engine.playSound(songs[currentSongIndex].name);
	}
	else if (keyboardActions.prevSong) {
		currentSongIndex--;
		if (currentSongIndex < 0)
			currentSongIndex = songs.size() - 1;
		if (engine.isPlaying(channelId))
			engine.stopChannel(channelId);
		engine.playSound(songs[currentSongIndex].name);
	}
	if (keyboardActions.togglePaused) {
		if (engine.isPlaying(channelId))
			engine.stopChannel(channelId);
		else
			engine.playSound(songs[currentSongIndex].name); // not sure how to restart after pause yet. this is a guess
	}
	if (keyboardActions.shuffleSongs) {
		bool wasPlaying = engine.isPlaying(channelId);
		if (wasPlaying)
			engine.stopChannel(channelId);
		shuffleSongs(songs); // current song can stay the same
		if (wasPlaying)
			engine.playSound(songs[currentSongIndex].name);
	}
	*/

	Input::getInstance().shutdownInput();

	std::cout << " sound over" << std::endl;

	return 0;
}
