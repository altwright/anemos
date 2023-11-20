#pragma once
#include <vulkan/vulkan.h>
#include "vkstate.h"
#include "int.h"
#include "vkshader.h"

VkCommandPool createCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags createFlags);
void recordModelDrawCommand(
    VkCommandBuffer commandBuffer, 
    VkRenderPass renderPass, 
    VkFramebuffer framebuffer, 
    PipelineDetails graphicsPipeline,
    VkExtent2D renderArea,
    PushConstant pushConstant,
    VkDescriptorSet descriptorSet,
    VkBuffer deviceBuffer,
    VkDeviceSize vertexBufferOffset,
    u32 vertexCount,
    VkDeviceSize indexBufferOffset,
    u32 indexCount);
void submitDrawCommand(
    VkQueue queue, 
    VkCommandBuffer commandBuffer, 
    VkSemaphore waitSemaphore,
    VkSemaphore signalSemaphore,
    VkFence hostFence);
VkResult presentSwapchain(
    VkQueue presentQueue, 
    VkSemaphore waitSemaphore,
    VkSwapchainKHR swapchain,
    uint32_t swapchainImageIndex);
VkCommandBuffer beginSingleTimeCommandBuffer(VkDevice device, VkCommandPool cmdPool);
void submitCommandBuffer(
    VkDevice device, 
    VkCommandPool cmdPool, 
    VkCommandBuffer cmdBuffer, 
    VkQueue queue);
FrameSynchroniser createFrameSynchroniser(VkDevice device);
VkCommandBuffer createPrimaryCommandBuffer(VkDevice device, VkCommandPool cmdPool);