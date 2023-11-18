#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "vkstate.h"

Buffer createDeviceBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);
Buffer createStagingBuffer(
    VmaAllocator allocator,
    VkDeviceSize bufferSize);
void copyToDeviceBuffer(
    size_t bytesCount,
    VkBuffer srcBuffer,
    size_t srcOffset,
    VkBuffer dstBuffer,
    size_t dstOffset,
    VkDevice device,
    VkCommandPool transferCmdPool,
    VkQueue transferQueue);