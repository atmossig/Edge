/*
 * EdgeCore.h
 * 
 * EDGE_ can be renamed to whatever you want.
 * but EDGE_ is apart of the core library.
 * 
 * Grant Abernathy
 * 
 * 06-14-2025
 *
 * A reusable, force-included core header.
 *
 * Responsibilities:
 * - Detect Compiler, Platform, Architecture, C++ Version, and Build Type.
 * - Provide global configuration for supported environments (platforms, compilers).
 * - Enforce C++ version requirements (min, max, preferred).
 * - Define standard utility macros like EDGE_ASSERT, EDGE_WARNING, EDGE_ERROR.
 * - Set up common preprocessor toggles for features like profiling.
 */

#include "EdgeAssert.h"

#ifndef INC_EDGE_CORE_
#define INC_EDGE_CORE_

#pragma once

//==================================================================================================
// 0. CORE
// 
// Core, top-level things, e.g., namespace.
//==================================================================================================
#define BEGIN_NS_EDGE namespace edge {
#define END_NS_EDGE }

//==================================================================================================
// 1. PROJECT-SPECIFIC PREPROCESSOR TOGGLES
// 
// These are intended to be defined in the build system (e.g., via -DEDGE_PROFILE=1).
// If they are not defined by the build system, they default to 0 (disabled).
//==================================================================================================
#ifndef EDGE_PROFILE
#define EDGE_PROFILE 0
#endif

#ifndef EDGE_TEST
#define EDGE_TEST 0
#endif

#ifndef EDGE_HACK
#define EDGE_HACK 0
#endif

#ifndef EDGE_TEMP_HACK
#define EDGE_TEMP_HACK 0
#endif

#ifndef EDGE_TEMP
#define EDGE_TEMP 0
#endif

#ifndef EDGE_DLL
#define EDGE_DLL 0
#endif

#ifndef EDGE_LIB
#define EDGE_LIB 0
#endif

//==================================================================================================
// 2. GLOBAL PROJECT CONFIGURATION
// 
// Define project-wide support for platforms, compilers, and C++ versions.
// 
// TO OVERRIDE: Define these in your build system's preprocessor definitions
// before this file is included.
//==================================================================================================

// --- Supported Platforms ---
#ifndef EDGE_GLOBAL_WIN_SUPPORTED
#define EDGE_GLOBAL_WIN_SUPPORTED 1 // 1 = Enabled, 0 = Disabled
#endif
#ifndef EDGE_GLOBAL_MAC_SUPPORTED
#define EDGE_GLOBAL_MAC_SUPPORTED 1
#endif
#ifndef EDGE_GLOBAL_IOS_SUPPORTED
#define EDGE_GLOBAL_IOS_SUPPORTED 1
#endif
#ifndef EDGE_GLOBAL_ANDROID_SUPPORTED
#define EDGE_GLOBAL_ANDROID_SUPPORTED 1
#endif

// --- Supported Compilers ---
#ifndef EDGE_GLOBAL_MSVC_SUPPORTED
#define EDGE_GLOBAL_MSVC_SUPPORTED 1
#endif
#ifndef EDGE_GLOBAL_CLANG_SUPPORTED
#define EDGE_GLOBAL_CLANG_SUPPORTED 1
#endif
#ifndef EDGE_GLOBAL_GCC_SUPPORTED
#define EDGE_GLOBAL_GCC_SUPPORTED 1
#endif

// --- C++ Version Requirements ---
// Values should match the __cplusplus standard values (e.g., 201703L for C++17)
#ifndef EDGE_GLOBAL_CPP_MIN_VERSION
#define EDGE_GLOBAL_CPP_MIN_VERSION 201402L			// C++14
#endif
#ifndef EDGE_GLOBAL_CPP_MAX_VERSION
#define EDGE_GLOBAL_CPP_MAX_VERSION 202002L			// C++20
#endif
#ifndef EDGE_GLOBAL_CPP_PREFERRED_VERSION
#define EDGE_GLOBAL_CPP_PREFERRED_VERSION 201703L	// C++17
#endif


//==================================================================================================
// 3. COMPILER DETECTION
// 
// Defines: EDGE_COMPILER_MSVC, EDGE_COMPILER_CLANG, EDGE_COMPILER_GCC
//          EDGE_COMPILER_VER_ (e.g., 1930 for MSVC 2022)
//==================================================================================================
#if defined(_MSC_VER)
#define EDGE_COMPILER_MSVC 1
#define EDGE_COMPILER_VER_ _MSC_VER
#elif defined(__clang__)
#define EDGE_COMPILER_CLANG 1
#define EDGE_COMPILER_VER_ (__clang_major__ * 100 + __clang_minor__)
#elif defined(__GNUC__)
#define EDGE_COMPILER_GCC 1
#define EDGE_COMPILER_VER_ (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#error "Unsupported compiler!"
#endif

// Define others as 0 for easy checking
#ifndef EDGE_COMPILER_MSVC
#define EDGE_COMPILER_MSVC 0
#endif
#ifndef EDGE_COMPILER_CLANG
#define EDGE_COMPILER_CLANG 0
#endif
#ifndef EDGE_COMPILER_GCC
#define EDGE_COMPILER_GCC 0
#endif


//==================================================================================================
// 4. PLATFORM & ARCHITECTURE DETECTION
// 
// Defines: EDGE_PLATFORM_WINDOWS, EDGE_PLATFORM_MACOS, EDGE_PLATFORM_IOS, EDGE_PLATFORM_ANDROID
//          EDGE_ARCH_X64, EDGE_ARCH_ARM64
//==================================================================================================

// --- Platform Detection ---
#if defined(_WIN64)
#define EDGE_PLATFORM_WINDOWS 1
#elif defined(_WIN32)
#error "Win32 is not a supported platform. Please compile for 64-bit (x64)."
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#define EDGE_PLATFORM_MACOS 1
#elif TARGET_OS_IPHONE
#define EDGE_PLATFORM_IOS 1
#else
#error "Unsupported Apple platform!"
#endif
#elif defined(__ANDROID__)
#define EDGE_PLATFORM_ANDROID 1
#else
#error "Unsupported platform!"
#endif

// Define others as 0 for easy checking
#ifndef EDGE_PLATFORM_WINDOWS
#define EDGE_PLATFORM_WINDOWS 0
#endif
#ifndef EDGE_PLATFORM_MACOS
#define EDGE_PLATFORM_MACOS 0
#endif
#ifndef EDGE_PLATFORM_IOS
#define EDGE_PLATFORM_IOS 0
#endif
#ifndef EDGE_PLATFORM_ANDROID
#define EDGE_PLATFORM_ANDROID 0
#endif

// --- Architecture Detection ---
#if defined(_M_X64) || defined(__x86_64__)
#define EDGE_ARCH_X64 1
#elif defined(_M_ARM64) || defined(__aarch64__)
#define EDGE_ARCH_ARM64 1
#else
#error "Unsupported architecture! Must be x64 or ARM64."
#endif

// Define others as 0 for easy checking
#ifndef EDGE_ARCH_X64
#define EDGE_ARCH_X64 0
#endif
#ifndef EDGE_ARCH_ARM64
#define EDGE_ARCH_ARM64 0
#endif


//==================================================================================================
// 5. GLOBAL SUPPORT VALIDATION
// 
// Checks if the current environment is supported by the project configuration.
// 
// Halts compilation if the current build environment is explicitly disabled.
//==================================================================================================
#if EDGE_COMPILER_MSVC && !EDGE_GLOBAL_MSVC_SUPPORTED
#error "This project has disabled support for the MSVC compiler."
#endif
#if EDGE_COMPILER_CLANG && !EDGE_GLOBAL_CLANG_SUPPORTED
#error "This project has disabled support for the Clang compiler."
#endif
#if EDGE_COMPILER_GCC && !EDGE_GLOBAL_GCC_SUPPORTED
#error "This project has disabled support for the GCC compiler."
#endif

#if EDGE_PLATFORM_WINDOWS && !EDGE_GLOBAL_WIN_SUPPORTED
#error "This project has disabled support for the Windows platform."
#endif
#if EDGE_PLATFORM_MACOS && !EDGE_GLOBAL_MAC_SUPPORTED
#error "This project has disabled support for the macOS platform."
#endif
#if EDGE_PLATFORM_IOS && !EDGE_GLOBAL_IOS_SUPPORTED
#error "This project has disabled support for the iOS platform."
#endif
#if EDGE_PLATFORM_ANDROID && !EDGE_GLOBAL_ANDROID_SUPPORTED
#error "This project has disabled support for the Android platform."
#endif


//==================================================================================================
// 6. C++ VERSION DETECTION
// 
// Defines: EDGE_CPP (e.g., 17, 20), EDGE_CPP_VERSION_LONG (e.g., 201703L)
//          Flags for each version like EDGE_CPP17, EDGE_CPP2017, etc.
//==================================================================================================

#if defined(_MSC_VER) && defined(_MSVC_LANG)
	// MSVC sometimes sets __cplusplus to an old value unless /Zc:__cplusplus is used.
	// _MSVC_LANG is the reliable source.
#define EDGE_CPP_VERSION_LONG _MSVC_LANG
#elif defined(__cplusplus)
#define EDGE_CPP_VERSION_LONG __cplusplus
#else
#error "Could not determine C++ version."
#endif

// Version-specific boolean flags (true if at least this version)
#define EDGE_CPP14 (EDGE_CPP_VERSION_LONG >= 201402L)
#define EDGE_CPP17 (EDGE_CPP_VERSION_LONG >= 201703L)
#define EDGE_CPP20 (EDGE_CPP_VERSION_LONG >= 202002L)
#define EDGE_CPP23 (EDGE_CPP_VERSION_LONG >= 202302L)

// Aliases with full year for clarity
#define EDGE_CPP2014 EDGE_CPP14
#define EDGE_CPP2017 EDGE_CPP17
#define EDGE_CPP2020 EDGE_CPP20
#define EDGE_CPP2023 EDGE_CPP23

// A simple integer representing the major C++ version
#if EDGE_CPP23
#define EDGE_CPP 23
#elif EDGE_CPP20
#define EDGE_CPP 20
#elif EDGE_CPP17
#define EDGE_CPP 17
#elif EDGE_CPP14
#define EDGE_CPP 14
#else
#define EDGE_CPP 11 // Fallback for C++11 or older
#endif


//==================================================================================================
// 7. C++ VERSION REQUIREMENT VALIDATION
// 
// Checks if the detected C++ version meets the project's global requirements.
//==================================================================================================
#if (EDGE_CPP_VERSION_LONG < EDGE_GLOBAL_CPP_MIN_VERSION)
#error "C++ version is too old! This project requires a newer C++ standard."
#endif

#if (EDGE_CPP_VERSION_LONG > EDGE_GLOBAL_CPP_MAX_VERSION)
#pragma message("Warning: Compiling with a C++ version newer than the maximum supported version specified by the project.")
#endif

#if (EDGE_CPP_VERSION_LONG != EDGE_GLOBAL_CPP_PREFERRED_VERSION)
#pragma message("Warning: This C++ version is not the project's preferred version.")
#endif


//==================================================================================================
// 8. BUILD TYPE DETECTION
// 
// Defines: EDGE_DEBUG, EDGE_RELEASE
//==================================================================================================
#if defined(_DEBUG) || (defined(DEBUG) && !defined(NDEBUG))
#define EDGE_DEBUG 1
#else
#define EDGE_RELEASE 1
#endif

#ifndef EDGE_DEBUG
#define EDGE_DEBUG 0
#endif
#ifndef EDGE_RELEASE
#define EDGE_RELEASE 0
#endif


//==================================================================================================
// 9. UTILITY MACROS
//==================================================================================================

// --- Compiler-Specific Helpers ---
#if EDGE_COMPILER_MSVC
#define EDGE_DEBUGBREAK() __debugbreak()
#else // Clang, GCC
#define EDGE_DEBUGBREAK() __builtin_trap()
#endif

// --- Compiler Messages (Warning and Error) ---
#define EDGE_STRINGIFY_IMPL(x) #x
#define EDGE_STRINGIFY(x) EDGE_STRINGIFY_IMPL(x)
#if EDGE_COMPILER_MSVC
	// MSVC uses #pragma message to output file/line info.
#define EDGE_WARNING(msg) __pragma(message(__FILE__ "(" EDGE_STRINGIFY(__LINE__) ") : Warning: " msg))
#define EDGE_ERROR(msg)   __pragma(message(__FILE__ "(" EDGE_STRINGIFY(__LINE__) ") : Error: " msg))
#else // GCC & Clang
	// GCC/Clang can use _Pragma to achieve a similar result.
#define EDGE_WARNING(msg) _Pragma(EDGE_STRINGIFY(GCC warning msg))
#define EDGE_ERROR(msg)   _Pragma(EDGE_STRINGIFY(GCC error msg))
#endif

#endif // INC_EDGE_CORE_