#include <utility>

//
// Created by Peter Eichinger on 30.10.18.
//

#pragma once

#include "saiga/vulkan/memory/MemoryLocation.h"

#include <list>
#include <memory>
#include <optional>
#include <ostream>
namespace Saiga::Vulkan::Memory
{
struct FreeListEntry
{
    vk::DeviceSize offset;
    vk::DeviceSize size;
    FreeListEntry() : offset(VK_WHOLE_SIZE), size(0) {}
    FreeListEntry(vk::DeviceSize _offset, vk::DeviceSize _size) : offset(_offset), size(_size) {}

    bool operator==(const FreeListEntry& rhs) const { return std::tie(offset, size) == std::tie(rhs.offset, rhs.size); }

    bool operator!=(const FreeListEntry& rhs) const { return !(rhs == *this); }

    friend std::ostream& operator<<(std::ostream& os, const FreeListEntry& entry)
    {
        os << "offset: " << entry.offset << " size: " << entry.size;
        return os;
    }

    inline vk::DeviceSize end() const { return offset + size; }
};



template <typename T>
struct SAIGA_VULKAN_API ChunkAllocation
{
    using FreeList      = std::vector<FreeListEntry>;
    using FreeIterator  = typename std::vector<FreeListEntry>::iterator;
    using AllocatedList = std::vector<std::unique_ptr<T>>;

    std::shared_ptr<Chunk> chunk;
    vk::Buffer buffer;
    AllocatedList allocations;
    FreeList freeList;
    std::optional<FreeListEntry> maxFreeRange;
    void* mappedPointer;

    vk::DeviceSize allocated;
    vk::DeviceSize size;
    ChunkAllocation() = default;

    ChunkAllocation(ChunkAllocation&&) = default;
    ChunkAllocation& operator=(ChunkAllocation&&) = default;
    ChunkAllocation(const ChunkAllocation&)       = delete;
    ChunkAllocation& operator=(const ChunkAllocation&) = delete;

    ChunkAllocation(std::shared_ptr<Chunk> _chunk, vk::Buffer _buffer, vk::DeviceSize _size, void* _mappedPointer)
        : chunk(std::move(_chunk)),
          buffer(_buffer),
          allocations(),
          freeList(),
          maxFreeRange(),
          mappedPointer(_mappedPointer),
          allocated(0),
          size(_size)
    {
        freeList.emplace_back(0, size);
        maxFreeRange = freeList.front();
    }

   public:
    inline vk::DeviceSize getFree() const { return size - allocated; }
};

template <typename T>
using ChunkContainer = std::vector<ChunkAllocation<T>>;

template <typename T>
using ChunkIterator = typename ChunkContainer<T>::iterator;

template <typename T>
using ConstChunkIterator = typename ChunkContainer<T>::const_iterator;

template <typename T>
using RevChunkIterator = typename ChunkContainer<T>::reverse_iterator;

template <typename T>
using ConstRevChunkIterator = typename ChunkContainer<T>::const_reverse_iterator;

template <typename T>
using AllocationIterator = typename ChunkAllocation<T>::AllocatedList::iterator;

template <typename T>
using ConstAllocationIterator = typename ChunkAllocation<T>::AllocatedList::const_iterator;

template <typename T>
using RevAllocationIterator = typename ChunkAllocation<T>::AllocatedList::reverse_iterator;

template <typename T>
using ConstRevAllocationIterator = typename ChunkAllocation<T>::AllocatedList::const_reverse_iterator;

template <typename T>
using FreeIterator = typename ChunkAllocation<T>::FreeList::iterator;

template <typename T>
using ConstFreeIterator = typename ChunkAllocation<T>::FreeList::const_iterator;
}  // namespace Saiga::Vulkan::Memory
