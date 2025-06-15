/*
 * EdgeMemory.h
 *
 * Grant Abernathy
 *
 * 06-14-2025
 *
 * Memory management system.
 *
 * Responsibilities:
 * - To provide memory allocation interfaces and utilities,
 * - provide support for memory tracking and debugging,
 * - Provide platform-specific memory optimizations,
 * - Provide alignment utilities for different data types,
 * - And provide memory pools and arena allocators.
 */

#ifndef INC_EDGE_CORE_MEMORY_
#define INC_EDGE_CORE_MEMORY_

#include "EdgeCore.h"
#include <cstddef>
#include <cstdint>
#include <new>
#include <limits>

BEGIN_NS_EDGE
BEGIN_NS_MEMORY

// Memory alignment constants
constexpr size_t EDGE_DEFAULT_ALIGNMENT = alignof(max_align_t);
constexpr size_t EDGE_SIMD_ALIGNMENT = 16;
constexpr size_t EDGE_CACHE_LINE_SIZE = 64;

// Memory allocation tags for tracking
enum class MemoryTag : uint8_t {
	NoTag = 0,
	//=============================================
	// Core level stuff
	//=============================================
	Foreground,		// anywhere the level is considered in bounds, that is foreground
	Background,		// anywhere the level is considered out of bounds, that is the background
	Interior,					// anywhere the level has an interior
	//=============================================
	// Animation
	//=============================================
	Animation,					// standard animation memory
	AnimationLocomotion,		// locomotion animation memory
	AnimationMotionMatching,	// motion matching memory
	//=============================================
	// Graphics
	//=============================================
	Particles,		// e.g., fire, smoke, rain, sparks
	Actors,			// e.g., physics props, placed props, characters, etc.
	//=============================================
	// Audio
	//=============================================
	AudioGlobal,	// global audio
	AudioSFX,		// sound effects
	AudioMusic,		// music
	AudioSpeech,	// dialogue
	AudioVox,		// non-dialogue character sounds
	//=============================================
	// AI
	//=============================================
	AI,				// global ai
	AITask,			// the ai task
	AIBrain,		// brain of the ai
	//=============================================
	// Misc shit
	//=============================================
	GUI,			// frontend
	Physics,		// physics system
	Cinematic,		// cinematic system
	Lighting,		// lighting
	Gameplay,		// gameplay related things
	Script,			// script related things
	Net,			// multiplayer networking
	Debug,			// debug memory
	Temp,			// temporary memory
	//=============================================
	COUNT			// Total
};

// Memory allocation stats
struct MemoryStats {
	size_t totalAllocated;
	size_t totalFreed;
	size_t currentUsage;
	size_t peakUsage;
	size_t allocationCount;
	size_t freeCount;

	MemoryStats() :
		totalAllocated(0),
		totalFreed(0),
		currentUsage(0),
		peakUsage(0),
		allocationCount(0),
		freeCount(0) {
	}
};

// Base memory allocator interface
class IAllocator {
public:
	virtual ~IAllocator() = default;

	virtual void* Allocate(size_t size, size_t alignment = EDGE_DEFAULT_ALIGNMENT) = 0;
	virtual void* Allocate(size_t size, MemoryTag tag, size_t alignment = EDGE_DEFAULT_ALIGNMENT) = 0;
	virtual void Free(void* ptr) = 0;
	virtual void GetStats(MemoryStats& stats) const = 0;
	virtual void Reset() = 0;
};

// System allocator that uses platform-specific memory allocation
class SystemAllocator : public IAllocator {
public:
	SystemAllocator();
	~SystemAllocator() override;

	void* Allocate(size_t size, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void* Allocate(size_t size, MemoryTag tag, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void Free(void* ptr) override;
	void GetStats(MemoryStats& stats) const override;
	void GetTagStats(MemoryTag tag, MemoryStats& stats) const;
	void Reset() override;

	// Enable/disable memory tracking
	void SetTrackingEnabled(bool enabled);
	bool IsTrackingEnabled() const;

	// Leak detection
	void ReportLeaks();

private:
	struct AllocationHeader;
	MemoryStats m_Stats;
	MemoryStats m_TagStats[static_cast<size_t>(MemoryTag::COUNT)];
	bool m_TrackingEnabled;

	// Linked list tracking
	AllocationHeader* m_FirstAllocation;
	AllocationHeader* m_LastAllocation;

	// Track an allocation in debug builds
	void TrackAllocation(AllocationHeader* header, size_t size, MemoryTag tag);
	void TrackDeallocation(AllocationHeader* header);
};

// Linear allocator - fast allocations, no individual frees
class LinearAllocator : public IAllocator {
public:
	LinearAllocator(size_t size);
	~LinearAllocator() override;

	void* Allocate(size_t size, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void* Allocate(size_t size, MemoryTag tag, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void Free(void* ptr) override; // No-op for individual elements
	void GetStats(MemoryStats& stats) const override;
	void Reset() override;

private:
	uint8_t* m_Buffer;
	size_t m_Size;
	size_t m_Offset;
	MemoryStats m_Stats;
};

// Pool allocator - fixed size allocations
class PoolAllocator : public IAllocator {
public:
	PoolAllocator(size_t elementSize, size_t elementCount, size_t alignment = EDGE_DEFAULT_ALIGNMENT);
	~PoolAllocator() override;

	void* Allocate(size_t size, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void* Allocate(size_t size, MemoryTag tag, size_t alignment = EDGE_DEFAULT_ALIGNMENT) override;
	void Free(void* ptr) override;
	void GetStats(MemoryStats& stats) const override;
	void Reset() override;

private:
	uint8_t* m_Buffer;
	uintptr_t* m_FreeList;
	size_t m_ElementSize;
	size_t m_ElementCount;
	size_t m_AlignedElementSize;
	size_t m_FreeCount;
	MemoryStats m_Stats;
};

// Global memory management functions
	// Initialize the memory subsystem
void Initialize();

// Shutdown the memory subsystem
void Shutdown();

// Get the default system allocator
SystemAllocator* GetSystemAllocator();

// Global memory allocation functions that use the system allocator
void* Allocate(size_t size, size_t alignment = EDGE_DEFAULT_ALIGNMENT);
void* AllocateAligned(size_t size, size_t alignment);
void* AllocateTagged(size_t size, MemoryTag tag, size_t alignment = EDGE_DEFAULT_ALIGNMENT);
void Free(void* ptr);
void FreeAligned(void* ptr);

// Utility functions
template<typename T, typename... Args>
T* New(Args&&... args) {
	void* ptr = Allocate(sizeof(T), alignof(T));
	return new(ptr) T(std::forward<Args>(args)...);
}

template<typename T>
void Delete(T* ptr) {
	if (ptr) {
		ptr->~T();
		Free(ptr);
	}
}

// Memory leak detection
void EnableTracking(bool enabled);
void ReportLeaks();

// Get memory stats
void GetStats(MemoryStats& stats);
void GetTagStats(MemoryTag tag, MemoryStats& stats);

// Alignment utilities
inline size_t AlignUp(size_t size, size_t alignment) {
	return (size + alignment - 1) & ~(alignment - 1);
}

inline void* AlignPointer(void* ptr, size_t alignment) {
	return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
}
END_NS_MEMORY
END_NS_EDGE

// Macros for memory allocation with automatic source tracking
#if EDGE_DEBUG
#define EDGE_NEW(Type, ...) new (Edge::Memory::AllocateTagged(sizeof(Type), Edge::MemoryTag::Default, alignof(Type))) Type(__VA_ARGS__)
#define EDGE_DELETE(ptr) Edge::Memory::Delete(ptr)
#define EDGE_MALLOC(size) Edge::Memory::Allocate(size)
#define EDGE_MALLOC_TAGGED(size, tag) Edge::Memory::AllocateTagged(size, tag)
#define EDGE_FREE(ptr) Edge::Memory::Free(ptr)
#else
#define EDGE_NEW(Type, ...) new Type(__VA_ARGS__)
#define EDGE_DELETE(ptr) delete ptr
#define EDGE_MALLOC(size) Edge::Memory::Allocate(size)
#define EDGE_MALLOC_TAGGED(size, tag) Edge::Memory::AllocateTagged(size, tag)
#define EDGE_FREE(ptr) Edge::Memory::Free(ptr)
#endif

#endif // INC_EDGE_CORE_MEMORY_