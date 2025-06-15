/*
 * EdgeMemory.cpp
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

#include "EdgeMemory.h"
#include <cstring>

#if EDGE_PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <malloc.h>
#endif

BEGIN_NS_EDGE
namespace memory {

	//==============================================================================
	// SystemAllocator
	//==============================================================================
	struct SystemAllocator::AllocationHeader
	{
		size_t size;
		size_t alignment;
		MemoryTag tag;
		const char* file;
		int line;
		AllocationHeader* prev;
		AllocationHeader* next;
		uint32_t guardValue;

		static constexpr uint32_t GUARD_VALUE = 0xDEADC0DE;
	};

	// Constructor for allocator
	SystemAllocator::SystemAllocator() : m_TrackingEnabled(EDGE_DEBUG ? true : false)
	{
		memset(&m_Stats, 0, sizeof(MemoryStats));
		memset(m_TagStats, 0, sizeof(m_TagStats));

		// Initialize tracking list
		m_FirstAllocation = nullptr;
		m_LastAllocation = nullptr;
	}

	// Deconstructor for allocation
	SystemAllocator::~SystemAllocator()
	{
		if (m_TrackingEnabled && m_Stats.allocationCount > m_Stats.freeCount)
		{
			ReportLeaks();
		}

		EDGE_ASSERT(m_Stats.allocationCount == m_Stats.freeCount, "Memory leak detected!");
	}

	void* SystemAllocator::Allocate(size_t size, size_t alignment)
	{
		return Allocate(size, MemoryTag::NoTag, alignment);
	}

	void* SystemAllocator::Allocate(size_t size, MemoryTag tag, size_t alignment)
	{
		if (size == 0)
		{
			return nullptr;
		}

		EDGE_ASSERT(alignment > 0 && (alignment & (alignment - 1)) == 0,
			"Alignment must be a power of two.");

		// For tracking, we want to allocate
		// extra space for our header.
		size_t totalSize = size;
		void* memory = nullptr;
		if (m_TrackingEnabled)
		{
			totalSize += sizeof(AllocationHeader);
		}

#if EDGE_PLATFORM_WINDOWS
		memory = _aligned_malloc(totalSize, alignment);
#else
		memory = aligned_alloc(alignment, Memory::AlignUp(totalSize, alignment));
#endif // EDGE_PLATFORM_WINDOWS

		if (!memory)
		{
			EDGE_ASSERT(false, "Memory allocation failed!");
			return nullptr;
		}

		void* userPtr = memory;

		if (m_TrackingEnabled)
		{
			// Setup allocation header
			AllocationHeader* header = static_cast<AllocationHeader*>(memory);
			header->size = size;
			header->alignment = alignment;
			header->tag = tag;
			header->file = nullptr;		// could retrieve from callstack if needed.
			header->line = 0;
			header->guardValue = AllocationHeader::GUARD_VALUE;
			header->prev = nullptr;
			header->next = nullptr;
			// Track this allocation
			TrackAllocation(header, size, tag);
			// User memory starts after header
			userPtr = reinterpret_cast<uint8_t*>(memory) + sizeof(AllocationHeader);
		}

		return userPtr;
	}

	void SystemAllocator::Free(void* ptr)
	{
		if (!ptr)
		{
			return;
		}

		void* actualPtr = ptr;

		if (m_TrackingEnabled)
		{
			// the header stored before the user pointer
			AllocationHeader* header = reinterpret_cast<AllocationHeader*>(
				reinterpret_cast<uint8_t*>(ptr) - sizeof(AllocationHeader));

			// verify guard value
			if (header->guardValue != AllocationHeader::GUARD_VALUE)
			{
				EDGE_ASSERT(false, "Memory corruption detected!");
				return;
			}

			TrackDeallocation(header);
			actualPtr = header;
		}

#if EDGE_PLATFORM_WINDOWS
		_aligned_free(actualPtr);
#else
		free(actualPtr);
#endif
	}

	void SystemAllocator::GetStats(MemoryStats& stats) const
	{
		stats = m_Stats;
	}

	void SystemAllocator::GetTagStats(MemoryTag tag, MemoryStats& stats) const
	{
		size_t tagIndex = static_cast<size_t>(tag);
		if (tagIndex < static_cast<size_t>(MemoryTag::COUNT)) {
			stats = m_TagStats[tagIndex];
		}
		else {
			memset(&stats, 0, sizeof(MemoryStats));
		}
	}

	void SystemAllocator::Reset()
	{
		EDGE_ASSERT(m_Stats.allocationCount == m_Stats.freeCount,
			"Cannot reset allocator with active allocations!");

		memset(&m_Stats, 0, sizeof(MemoryStats));
		memset(m_TagStats, 0, sizeof(m_TagStats));

		// Reset tracking list
		m_FirstAllocation = nullptr;
		m_LastAllocation = nullptr;
	}

	void SystemAllocator::SetTrackingEnabled(bool enabled) {
		EDGE_ASSERT(m_Stats.allocationCount == m_Stats.freeCount,
			"Cannot change tracking state with active allocations!");

		m_TrackingEnabled = enabled;
	}

	bool SystemAllocator::IsTrackingEnabled() const
	{
		return m_TrackingEnabled;
	}

	// Implementation using a linked list to track all allocations
	void SystemAllocator::TrackAllocation(AllocationHeader* header, size_t size, MemoryTag tag)
	{
		// Update global stats
		m_Stats.totalAllocated += size;
		m_Stats.currentUsage += size;
		m_Stats.allocationCount++;

		if (m_Stats.currentUsage > m_Stats.peakUsage) {
			m_Stats.peakUsage = m_Stats.currentUsage;
		}

		// Update tag-specific stats
		size_t tagIndex = static_cast<size_t>(tag);
		if (tagIndex < static_cast<size_t>(MemoryTag::COUNT)) {
			m_TagStats[tagIndex].totalAllocated += size;
			m_TagStats[tagIndex].currentUsage += size;
			m_TagStats[tagIndex].allocationCount++;

			if (m_TagStats[tagIndex].currentUsage > m_TagStats[tagIndex].peakUsage) {
				m_TagStats[tagIndex].peakUsage = m_TagStats[tagIndex].currentUsage;
			}
		}

		// Add to linked list for tracking
		if (m_FirstAllocation == nullptr) {
			m_FirstAllocation = header;
			m_LastAllocation = header;
		}
		else {
			header->prev = m_LastAllocation;
			m_LastAllocation->next = header;
			m_LastAllocation = header;
		}
	}

	void SystemAllocator::TrackDeallocation(AllocationHeader* header) {
		// Update global stats
		m_Stats.totalFreed += header->size;
		m_Stats.currentUsage -= header->size;
		m_Stats.freeCount++;

		// Update tag-specific stats
		size_t tagIndex = static_cast<size_t>(header->tag);
		if (tagIndex < static_cast<size_t>(MemoryTag::COUNT)) {
			m_TagStats[tagIndex].totalFreed += header->size;
			m_TagStats[tagIndex].currentUsage -= header->size;
			m_TagStats[tagIndex].freeCount++;
		}

		// Remove from tracking list
		if (header->prev) {
			header->prev->next = header->next;
		}
		else {
			m_FirstAllocation = header->next;
		}

		if (header->next) {
			header->next->prev = header->prev;
		}
		else {
			m_LastAllocation = header->prev;
		}
	}

	void SystemAllocator::ReportLeaks() {
		if (!m_TrackingEnabled || m_Stats.allocationCount == m_Stats.freeCount) {
			return;
		}

		// Iterate through the linked list of allocation headers to report each leak's details
		EDGE_WARNING("Memory leaks detected!");

		printf("Total leaked memory: %zu bytes\n",
			m_Stats.totalAllocated - m_Stats.totalFreed);
		printf("Total allocations: %zu, frees: %zu\n",
			m_Stats.allocationCount, m_Stats.freeCount);

		// Report tag-specific leaks
		for (size_t i = 0; i < static_cast<size_t>(MemoryTag::COUNT); ++i) {
			if (m_TagStats[i].allocationCount > m_TagStats[i].freeCount) {
				printf("Tag %zu: Leaked %zu bytes in %zu blocks\n", i,
					m_TagStats[i].totalAllocated - m_TagStats[i].totalFreed,
					m_TagStats[i].allocationCount - m_TagStats[i].freeCount);
			}
		}

		// Report individual leaks
		printf("\nDetailed leak report:\n");
		printf("-------------------------------------------------\n");

		AllocationHeader* current = m_FirstAllocation;
		int leakCount = 0;

		while (current) {
			leakCount++;
			printf("Leak #%d: %zu bytes (tag: %d)",
				leakCount,
				current->size,
				static_cast<int>(current->tag));

			if (current->file) {
				printf(", allocated at %s:%d", current->file, current->line);
			}

			printf("\n");
			current = current->next;
		}

		printf("-------------------------------------------------\n");
		printf("Total leaks found: %d\n", leakCount);
	}

	//==================================================================================================
	// LinearAllocator Implementation
	//==================================================================================================

	LinearAllocator::LinearAllocator(size_t size)
		: m_Size(size), m_Offset(0) {

		m_Buffer = static_cast<uint8_t*>(Memory::Allocate(size, EDGE_CACHE_LINE_SIZE));
		EDGE_ASSERT(m_Buffer != nullptr, "Failed to allocate memory for LinearAllocator");
		memset(&m_Stats, 0, sizeof(MemoryStats));
	}

	LinearAllocator::~LinearAllocator() {
		Memory::Free(m_Buffer);
		m_Buffer = nullptr;
	}

	void* LinearAllocator::Allocate(size_t size, size_t alignment) {
		return Allocate(size, MemoryTag::NoTag, alignment);
	}

	void* LinearAllocator::Allocate(size_t size, MemoryTag tag, size_t alignment) {
		if (size == 0) {
			return nullptr;
		}

		// Calculate aligned address
		size_t alignmentMask = alignment - 1;
		size_t alignedOffset = (m_Offset + alignmentMask) & ~alignmentMask;

		if (alignedOffset + size > m_Size) {
			EDGE_ASSERT(false, "LinearAllocator out of memory");
			return nullptr;
		}

		void* ptr = m_Buffer + alignedOffset;
		m_Offset = alignedOffset + size;

		// Update stats
		m_Stats.totalAllocated += size;
		m_Stats.currentUsage += size;
		m_Stats.allocationCount++;

		if (m_Stats.currentUsage > m_Stats.peakUsage) {
			m_Stats.peakUsage = m_Stats.currentUsage;
		}

		return ptr;
	}

	void LinearAllocator::Free(void* ptr) {
		// Individual frees are not supported in a linear allocator
		// We simply ignore this call
		(void)ptr;
	}

	void LinearAllocator::GetStats(MemoryStats& stats) const {
		stats = m_Stats;
	}

	void LinearAllocator::Reset() {
		m_Offset = 0;
		m_Stats.totalFreed += m_Stats.currentUsage;
		m_Stats.currentUsage = 0;
		m_Stats.freeCount += m_Stats.allocationCount;
	}

	//==================================================================================================
	// PoolAllocator Implementation
	//==================================================================================================

	PoolAllocator::PoolAllocator(size_t elementSize, size_t elementCount, size_t alignment)
		: m_ElementSize(elementSize), m_ElementCount(elementCount), m_FreeCount(elementCount) {

		// Calculate aligned element size
		m_AlignedElementSize = Memory::AlignUp(elementSize, alignment);

		// Allocate the pool
		size_t totalSize = m_AlignedElementSize * elementCount;
		m_Buffer = static_cast<uint8_t*>(Memory::Allocate(totalSize, alignment));
		EDGE_ASSERT(m_Buffer != nullptr, "Failed to allocate memory for PoolAllocator");

		// Setup free list
		m_FreeList = reinterpret_cast<uintptr_t*>(m_Buffer);

		for (size_t i = 0; i < elementCount - 1; ++i) {
			uintptr_t* block = reinterpret_cast<uintptr_t*>(m_Buffer + (i * m_AlignedElementSize));
			*block = reinterpret_cast<uintptr_t>(m_Buffer + ((i + 1) * m_AlignedElementSize));
		}

		// Last block points to nullptr
		uintptr_t* lastBlock = reinterpret_cast<uintptr_t*>(m_Buffer + ((elementCount - 1) * m_AlignedElementSize));
		*lastBlock = 0;

		memset(&m_Stats, 0, sizeof(MemoryStats));
	}

	PoolAllocator::~PoolAllocator() {
		Memory::Free(m_Buffer);
		m_Buffer = nullptr;
		m_FreeList = nullptr;
	}

	void* PoolAllocator::Allocate(size_t size, size_t alignment) {
		return Allocate(size, MemoryTag::NoTag, alignment);
	}

	void* PoolAllocator::Allocate(size_t size, MemoryTag tag, size_t alignment) {
		if (size > m_ElementSize) {
			EDGE_ASSERT(false, "Requested size is larger than pool element size");
			return nullptr;
		}

		if (m_FreeList == nullptr) {
			EDGE_ASSERT(false, "Pool allocator is out of memory");
			return nullptr;
		}

		void* ptr = m_FreeList;
		m_FreeList = reinterpret_cast<uintptr_t*>(*m_FreeList);
		m_FreeCount--;

		// Update stats
		m_Stats.totalAllocated += m_ElementSize;
		m_Stats.currentUsage += m_ElementSize;
		m_Stats.allocationCount++;

		if (m_Stats.currentUsage > m_Stats.peakUsage) {
			m_Stats.peakUsage = m_Stats.currentUsage;
		}

		return ptr;
	}

	void PoolAllocator::Free(void* ptr) {
		if (ptr == nullptr) {
			return;
		}

		// Validate the pointer is within our pool
		if (ptr < m_Buffer || ptr >= (m_Buffer + m_AlignedElementSize * m_ElementCount)) {
			EDGE_ASSERT(false, "Pointer does not belong to this pool");
			return;
		}

		// Insert at the head of free list
		*reinterpret_cast<uintptr_t*>(ptr) = reinterpret_cast<uintptr_t>(m_FreeList);
		m_FreeList = reinterpret_cast<uintptr_t*>(ptr);
		m_FreeCount++;

		// Update stats
		m_Stats.totalFreed += m_ElementSize;
		m_Stats.currentUsage -= m_ElementSize;
		m_Stats.freeCount++;
	}

	void PoolAllocator::GetStats(MemoryStats& stats) const {
		stats = m_Stats;
	}

	void PoolAllocator::Reset() {
		// Reinitialize the free list
		for (size_t i = 0; i < m_ElementCount - 1; ++i) {
			uintptr_t* block = reinterpret_cast<uintptr_t*>(m_Buffer + (i * m_AlignedElementSize));
			*block = reinterpret_cast<uintptr_t>(m_Buffer + ((i + 1) * m_AlignedElementSize));
		}

		// Last block points to nullptr
		uintptr_t* lastBlock = reinterpret_cast<uintptr_t*>(m_Buffer + ((m_ElementCount - 1) * m_AlignedElementSize));
		*lastBlock = 0;

		m_FreeList = reinterpret_cast<uintptr_t*>(m_Buffer);
		m_FreeCount = m_ElementCount;

		// Update stats
		m_Stats.totalFreed += m_Stats.currentUsage;
		m_Stats.currentUsage = 0;
		m_Stats.freeCount += m_Stats.allocationCount;
	}

	//==================================================================================================
	// Global Memory Management Functions
	//==================================================================================================

	// Global system allocator instance
	static SystemAllocator* g_SystemAllocator = nullptr;

	namespace Memory {

		void Initialize() {
			if (g_SystemAllocator == nullptr) {
				// Use standard new/delete to create system allocator
				g_SystemAllocator = new SystemAllocator();
			}
		}

		void Shutdown() {
			if (g_SystemAllocator) {
				delete g_SystemAllocator;
				g_SystemAllocator = nullptr;
			}
		}

		SystemAllocator* GetSystemAllocator() {
			if (!g_SystemAllocator) {
				Initialize();
			}
			return g_SystemAllocator;
		}

		void* Allocate(size_t size, size_t alignment)
		{
			return GetSystemAllocator()->Allocate(size, alignment);
		}

		void* AllocateAligned(size_t size, size_t alignment)
		{
			return GetSystemAllocator()->Allocate(size, alignment);
		}

		void* AllocateTagged(size_t size, MemoryTag tag, size_t alignment)
		{
			return GetSystemAllocator()->Allocate(size, tag, alignment);
		}

		void Free(void* ptr)
		{
			GetSystemAllocator()->Free(ptr);
		}

		void FreeAligned(void* ptr)
		{
			GetSystemAllocator()->Free(ptr);
		}

		void EnableTracking(bool enabled)
		{
			GetSystemAllocator()->SetTrackingEnabled(enabled);
		}

		void ReportLeaks()
		{
			GetSystemAllocator()->ReportLeaks();
		}

		void GetStats(MemoryStats& stats)
		{
			GetSystemAllocator()->GetStats(stats);
		}

		void GetTagStats(MemoryTag tag, MemoryStats& stats) {
			// Access the tag-specific stats from the allocator
			GetSystemAllocator()->GetTagStats(tag, stats);
		}

	}
}
END_NS_EDGE