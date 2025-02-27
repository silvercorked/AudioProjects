#pragma once

#include "PrimitiveTypes.hpp"

#include <iostream>
#include <format>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

auto eraseLines(i32 count) -> void {
	if (count > 0) {
		while (count-- > 0) {
			std::cout << "\x1b[2K" // delete line
				<< "\x1b[1A"; // move cursor up one
		}
		std::cout << "\r";
	}
}

auto moveCursorUpLines(i32 count) -> void {
	while (count-- > 0) {
		std::cout << "\x1b[1A";
	}
}

//https://www.geeksforgeeks.org/how-to-detect-keypress-in-windows-using-cpp/
auto [[nodiscard]] checkIfKeyPressed(const int keycode) -> bool {
	if (GetAsyncKeyState(keycode) & 0x8000) {
		//std::cout << std::format("Pressed Key: {} (ASCII value: {})\n",
		//	static_cast<char>(keycode), keycode
		//);
		return true;
	}
	return false;
}
