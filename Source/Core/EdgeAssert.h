/*
 * EdgeAssert.h
 *
 * Grant Abernathy
 *
 * 06-15-2025
 *
 * Assertion system for runtime validation and debugging.
 *
 * Responsibilities:
 * - Provide runtime assertion capabilities with customizable behavior
 * - Support for different assertion levels and types
 * - Ability to capture file, line, and message information
 * - Configurable assertion handlers for custom behavior
 */

#ifndef INC_EDGE_CORE_ASSERT_
#define INC_EDGE_CORE_ASSERT_

#include "EdgeCore.h"
#include <string>
#include <functional>

BEGIN_NS_EDGE
BEGIN_NS_ASSERT

// Forward declarations
enum class AssertLevel;
struct AssertInfo;
class AssertHandler;

enum class AssertLevel {
	Info,		// Info, doesn't break or do anything
	Warning,	// Warning, continues execution
	Error,		// Error, breaks in the debugger
	Fatal		// Fatal, always breaks
};

// Info about an assertion
struct AssertInfo {
	const char* condition;			// Condition that failed the assertion
	const char* message;			// Message for the assertion
	const char* file;				// File where the assertion occurred
	int line;						// Line number of the assertion
	AssertLevel level;				// Severity level of the assertion
};

// Function type for custom assert handlers
using AssertCallbackFn = std::function<void(const AssertInfo&)>;

// Assert handler class - allows for custom assertion behavior
class AssertHandler {
public:
	static AssertHandler& Get();

	// Register a custom handler
	void SetCallback(AssertCallbackFn callback);

	// Reset to default handler
	void ResetCallback();

	// Handle an assertion
	bool HandleAssert(const AssertInfo& info);

private:
	AssertHandler() = default;
	AssertCallbackFn m_Callback;

	// Default handler behavior
	bool DefaultHandler(const AssertInfo& info);
};

// Format an assertion message
std::string FormatAssertMessage(const AssertInfo& info);

// Break into the debugger if available
void DebugBreak();

END_NS_ASSERT
END_NS_EDGE

//==================================================================================================
// Assertion Macros
//==================================================================================================

// Core assert implementation - do not use directly, use the macros below
#define EDGE_ASSERT_IMPL(condition, level, message, ...) \
	do { \
		if (!(condition)) { \
			edge::assert::AssertInfo info { \
				#condition, \
				message, \
				__FILE__, \
				__LINE__, \
				level \
			}; \
			if (edge::assert::AssertHandler::Get().HandleAssert(info)) { \
				edge::assert::DebugBreak(); \
			} \
		} \
	} while(0)

// Main assertion macro - breaks in debug mode if condition is false
#if EDGE_DEBUG
#define EDGE_ASSERT(condition, message, ...) \
		EDGE_ASSERT_IMPL(condition, edge::assert::AssertLevel::Error, message, ##__VA_ARGS__)
#else
#define EDGE_ASSERT(condition, message, ...) ((void)0)
#endif

// Always evaluated assertion - useful for checking return values
#define EDGE_VERIFY(condition, message, ...) \
	EDGE_ASSERT_IMPL(condition, edge::assert::AssertLevel::Error, message, ##__VA_ARGS__)

// Fatal assertion - always breaks, even in release builds
#define EDGE_ASSERT_FATAL(condition, message, ...) \
	EDGE_ASSERT_IMPL(condition, edge::assert::AssertLevel::Fatal, message, ##__VA_ARGS__)

// Warning assertion - logs but doesn't break
#define EDGE_ASSERT_WARN(condition, message, ...) \
	EDGE_ASSERT_IMPL(condition, edge::assert::AssertLevel::Warning, message, ##__VA_ARGS__)

// Message - always displays a message but never breaks
#define EDGE_ASSERT_MESSAGE(message, ...) \
	EDGE_ASSERT_IMPL(true, edge::assert::AssertLevel::Info, message, ##__VA_ARGS__)

#endif // INC_EDGE_CORE_ASSERT_