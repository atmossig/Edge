/*
 * EdgeAssert.cpp
 *
 * Grant Abernathy
 *
 * 06-15-2025
 *
 * Assertion system for runtime validation and debugging.
 *
 */

#include "EdgeAssert.h"
#include <iostream>
#include <sstream>
#include <cstdarg>

#if EDGE_PLATFORM_WINDOWS
#include <Windows.h>
#endif

BEGIN_NS_EDGE
namespace assert {
	//====================
	// AssertHandler
	//====================
	AssertHandler& AssertHandler::Get() {
		static AssertHandler instance;
		return instance;
	}

	void AssertHandler::SetCallback(AssertCallbackFn callback) {
		m_Callback = callback;
	}

	void AssertHandler::ResetCallback() {
		m_Callback = nullptr;
	}

	bool AssertHandler::HandleAssert(const AssertInfo& info) {
		// If a custom callback is registered, use it.
		if (m_Callback) {
			m_Callback(info);
			// We will still use the default behavior for deciding whether to break
			return DefaultHandler(info);
		}

		// Default handling behavior
		return DefaultHandler(info);
	}

	bool AssertHandler::DefaultHandler(const AssertInfo& info) {
		// Format and display the message
		std::string formattedMessage = FormatAssertMessage(info);

		// Output to standard error
		std::cerr << formattedMessage << std::endl;

		// Output to debug console on Windows
#if EDGE_PLATFORM_WINDOWS && EDGE_DEBUG
		OutputDebugStringA(formattedMessage.c_str());
		OutputDebugStringA("\n");
#endif

		// Determine if we should break based on level and build type
		switch (info.level) {
		case AssertLevel::Fatal:
			return true; // Always break

		case AssertLevel::Error:
			return EDGE_DEBUG; // Break in debug builds

		case AssertLevel::Warning:
		case AssertLevel::Info:
		default:
			return false; // Don't break
		}
	}

	std::string FormatMessage(const AssertInfo& info) {
		std::stringstream ss;

		// Add assertion level indicator
		switch (info.level) {
		case AssertLevel::Fatal:
			ss << "[FATAL] ";
			break;
		case AssertLevel::Error:
			ss << "[ERROR] ";
			break;
		case AssertLevel::Warning:
			ss << "[WARNING] ";
			break;
		case AssertLevel::Info:
			ss << "[INFO] ";
			break;
		}

		// Add base assertion information
		ss << "Assertion Failed: " << info.condition;

		// Add custom message
		if (info.message && info.message[0] != '\0') {
			ss << " (" << info.message << ")";
		}

		// Add file and line information
		ss << "\n  at " << info.file << ":" << info.line;

		return ss.str();
	}

	void DebugBreak() {
#if EDGE_COMPILER_MSVC
		__debugbreak();
#elif EDGE_COMPILER_CLANG || EDGE_COMPILER_GCC
		__builtin_trap();
#else
		// Fallback mechanism if no specific debugbreak is available
		abort();
#endif
	}

}
END_NS_EDGE