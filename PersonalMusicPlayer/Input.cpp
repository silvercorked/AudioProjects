
#include "Input.hpp"

#include <ranges>
#include <algorithm>
#include <iostream>
#include <format>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windef.h>

Input::Input() :
	keyCallbacks{},
	keyMap{},
	heldKeysMemory{},
	shutdown{false}
{
	heldKeysMemory.fill(false);

	this->inputThread = std::make_unique<std::jthread>(
		[this]() {
			this->inputThreadFunction();
		}
	);
}
auto Input::isPressed(i32 keycode) -> bool {
	keycode = std::toupper(keycode);
	std::lock_guard<std::mutex> lock(this->modificationLock);
	auto foundIter = this->keyMap.find(keycode);
	if (foundIter != this->keyMap.end()) {
		if (this->heldKeysMemory[static_cast<i32>(foundIter->second)])
			return true;
	}
	return false;
}
auto Input::registerKeyToAction(i32 keycode, KeyActions action) -> bool {
	keycode = std::toupper(keycode);
	std::lock_guard<std::mutex> lock(this->modificationLock);
	auto foundIter = this->keyMap.find(keycode);
	if (foundIter != this->keyMap.end())
		return false;
	this->keyMap[keycode] = action;
	return true;
}
auto Input::subscribeToKeypress(std::function<void()> callback, KeyActions action) -> i64 {
	std::lock_guard<std::mutex> lock(this->modificationLock);
	this->keyCallbacks[action].push_back(callback);
	i32 key = -1;
	for (auto [k, act] : this->keyMap) {
		if (act == action)
			key = k;
	}
	return this->generateCallbackId(this->keyCallbacks[action].size() - 1, key);
}
auto Input::unsubscribeCallback(i64 callbackId) -> bool {
	auto decodedId = this->decodeCallbackId(callbackId);
	std::lock_guard<std::mutex> lock(this->modificationLock);
	auto actionIter = this->keyMap.find(decodedId.first);
	if (actionIter != this->keyMap.end()) {
		auto foundIter = this->keyCallbacks.find(actionIter->second);
		if (
			foundIter != this->keyCallbacks.end() &&
			decodedId.second < foundIter->second.size()
		) {
			foundIter->second.erase(foundIter->second.begin() + decodedId.second);
			return true;
		}
	}
	
	return false; // failed to delete
}
auto Input::unsubscribeAllCallbacks() -> void {
	std::lock_guard<std::mutex> lock(this->modificationLock);
	this->keyCallbacks.clear();
}

auto Input::triggerCallbacks(KeyActions action) -> void {
	std::lock_guard<std::mutex> lock(this->modificationLock);
	auto foundIter = this->keyCallbacks.find(action);
	if (foundIter != this->keyCallbacks.end()) {
		for (auto keyFunc : (*foundIter).second)
			keyFunc();
	}
}

auto Input::shutdownInput() -> void {
	this->shutdown = true;
}

auto Input::generateCallbackId(i32 position, i32 keycode) -> i64 {
	i64 id = 0; // id is packed keycode and position. if no keycode associated, -1
	id |= (static_cast<i64>(keycode) << 32);
	id |= position;
	return id;
}
auto Input::decodeCallbackId(i64 id) -> std::pair<i32, i32> {
	return std::make_pair<i32, i32>(
		static_cast<i32>((id & 0xFFFFFFFF00000000) >> 32),
		static_cast<i32>(id & 0x00000000FFFFFFFF)
	);
}
/*
checkIfKeyPressed uses GetAsyncKeyState which can tell you if a key is pressed
and if that key press is new since the last call. However, the second behavior, being
able to tell if the key press is new, depends on which process calls GetAsyncKeyState first.
So that second behavior is unreliable. Instead I can hold a bool array for each key I want to keep track of
and use it to tell if a key press is unique. Each unique one is caught in KeyboardActions, which
let's it be moved around easily so it can get handled.
*/
auto Input::checkKeyboardInput() -> KeyboardActions {
	constexpr const auto checkIfKeyPressed = [](const int keycode) -> bool {
		HWND con = GetConsoleWindow(); // console window handle (kinda deprecated but works for now)
		HWND fore = GetForegroundWindow(); // whatever window has focus (if con == fore, we have focus)
		return (GetAsyncKeyState(keycode) & 0x8000) && con == fore && con != NULL; // only want focused inputs
	};

	std::lock_guard<std::mutex> lock(this->modificationLock);
	KeyboardActions curr{};
	for (auto [keyCode, action] : this->keyMap) {
		if (checkIfKeyPressed(keyCode)) {
			bool& associatedHeldMem = this->heldKeysMemory[static_cast<i32>(action)];
			if (associatedHeldMem)
				curr.held.push_back(action);
			else {
				curr.pressed.push_back(action);
				associatedHeldMem = true;
			}
		}
	}

	for (auto i = 0; i < this->heldKeysMemory.size(); i++) {
		bool& associatedHeldMem = this->heldKeysMemory[i];
		if (associatedHeldMem) {
			bool pressed = std::any_of(
				curr.pressed.cbegin(),
				curr.pressed.cend(),
				[i](KeyActions k) {
					return i == static_cast<i32>(k);
				}
			);
			bool held = std::any_of(
				curr.held.cbegin(),
				curr.held.cend(),
				[i](KeyActions k) {
					return i == static_cast<i32>(k);
				}
			);
			if (!(pressed || held)) {
				associatedHeldMem = false;
				curr.released.push_back(static_cast<KeyActions>(i));
			}
		}
	}

	return curr;
}
auto Input::inputThreadFunction() -> void {
	while (!this->shutdown) {
		auto keyboardActions = this->checkKeyboardInput();

		for (auto i = 0; i < keyboardActions.pressed.size(); i++) {
			//std::cout << std::format(
			//	"Key Action Pressed: {}, Active == Foreground: {}, Non-Null: {}\n",
			//	static_cast<i32>(keyboardActions.pressed[i]),
			//	(GetConsoleWindow() == GetForegroundWindow() ? "same" : "different"),
			//	(GetConsoleWindow() != NULL ? "not null" : "null")
			//);
			this->triggerCallbacks(keyboardActions.pressed[i]);
		}
		
		/*
		for (auto i = 0; i < keyboardActions.held.size(); i++) {
			std::cout << std::format(
				"Key Action Held: {}\n",
				static_cast<i32>(keyboardActions.held[i])
			);
		}
		for (auto i = 0; i < keyboardActions.released.size(); i++) {
			std::cout << std::format(
				"Key Action Released: {}\n",
				static_cast<i32>(keyboardActions.released[i])
			);
		}
		*/

		

		/*
		if (keyboardActions.nextSong || keyboardActions.prevSong ||
			keyboardActions.togglePaused || keyboardActions.shuffleSongs ||
			keyboardActions.quitApplication) {
			std::cout << std::format(
				"nextSong: {}, prevSong: {}, togglePaused: {}, shuffleSongs: {}, quit: {}\n",
				(keyboardActions.nextSong ? "true" : "false"),
				(keyboardActions.prevSong ? "true" : "false"),
				(keyboardActions.togglePaused ? "true" : "false"),
				(keyboardActions.shuffleSongs ? "true" : "false"),
				(keyboardActions.quitApplication ? "true" : "false")
			);
		}
		*/
	}
}
