
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

	Input::getInstance().registerKeyToAction('N', KeyActions::nextSong);
	Input::getInstance().registerKeyToAction('B', KeyActions::prevSong);
	Input::getInstance().registerKeyToAction('P', KeyActions::togglePaused);
	Input::getInstance().registerKeyToAction('S', KeyActions::shuffleSongs);
	Input::getInstance().registerKeyToAction('Q', KeyActions::quitApplication);

	auto songs = PersonalMusicPlayer::loadEntireLibrary(engine);
	std::cout << "\n\n";
	LoadedSong playingSong;

	PersonalMusicPlayer::shuffleSongs(songs);

	i32 currentSongIndex = 0;
	bool quit = false;

	i32 channelId = engine.playSound(songs[currentSongIndex].name);
	playingSong = LoadedSong(songs[currentSongIndex], channelId);

	std::mutex audioMutex;

	Input::getInstance().subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			currentSongIndex++;
			if (currentSongIndex >= songs.size())
				currentSongIndex = 0;
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			i32 newChannelId = engine.playSound(songs[currentSongIndex].name);
			playingSong = LoadedSong(songs[currentSongIndex], newChannelId);
		}, KeyActions::nextSong
	);
	Input::getInstance().subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			currentSongIndex--;
			if (currentSongIndex < 0)
				currentSongIndex = songs.size() - 1;
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			i32 newChannelId = engine.playSound(songs[currentSongIndex].name);
			playingSong = LoadedSong(songs[currentSongIndex], newChannelId);
		}, KeyActions::prevSong
	);
	Input::getInstance().subscribeToKeypress(
		[&quit]() -> void {
			quit = true;
		}, KeyActions::quitApplication
	);
	Input::getInstance().subscribeToKeypress(
		[&engine, &currentSongIndex, &songs, &playingSong, &audioMutex]() -> void {
			std::lock_guard<std::mutex> lock(audioMutex);
			PersonalMusicPlayer::shuffleSongs(songs);
			if (engine.isPlaying(playingSong.channelId))
				engine.stopChannel(playingSong.channelId);
			i32 newChannelid = engine.playSound(songs[currentSongIndex].name);
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
					currentSongIndex++;
					if (currentSongIndex >= songs.size())
						currentSongIndex = 0;
					if (engine.isPlaying(playingSong.channelId))
						engine.stopChannel(playingSong.channelId);
					i32 newChannelid = engine.playSound(songs[currentSongIndex].name);
					playingSong = LoadedSong(songs[currentSongIndex], newChannelid);
				}
				if (quit) {
					engine.stopAllChannels();
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
