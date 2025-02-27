#pragma once

#include "PrimitiveTypes.hpp"

#include <unordered_map>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <type_traits>
#include <array>

const enum struct KeyActions : i32 {
	nextSong = 0,
	prevSong = 1,
	togglePaused = 2,
	shuffleSongs = 3,
	quitApplication = 4,
	MAX_SIZE = 5
};

struct KeyboardActions {
	std::vector<KeyActions> pressed;
	std::vector<KeyActions> held;
	std::vector<KeyActions> released;
};

class Input {
public:
	static Input& getInstance() {
		static Input instance;
		return instance;
	}
private:
	Input();
public:
	Input(const Input&) = delete;
	void operator=(const Input&) = delete;

private:
	std::unordered_map<i32, KeyActions> keyMap;
	std::unordered_map<KeyActions, std::vector<std::function<void()>>> keyCallbacks;
	std::array<bool, static_cast<i32>(KeyActions::MAX_SIZE)> heldKeysMemory;
	std::unique_ptr<std::jthread> inputThread;
	std::mutex modificationLock;
	bool shutdown;

public:
	auto isPressed(i32 keycode) -> bool;
	auto registerKeyToAction(i32 keycode, KeyActions action) -> bool;
	auto subscribeToKeypress(std::function<void()> callback, KeyActions action) -> i64;
	auto unsubscribeCallback(i64 callbackId) -> bool;
	auto unsubscribeAllCallbacks() -> void;
	auto triggerCallbacks(KeyActions action) -> void;
	auto shutdownInput() -> void;

private:
	auto generateCallbackId(i32 position, i32 keycode = -1) -> i64;
	auto decodeCallbackId(i64 id) -> std::pair<i32, i32>;
	auto checkKeyboardInput() -> KeyboardActions;
	auto inputThreadFunction() -> void;
};
