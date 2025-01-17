#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "vkstate.h"

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatsCount;
    VkSurfaceFormatKHR *formats;//free
    uint32_t presentModesCount;
    VkPresentModeKHR *presentModes;//free
} SwapchainSupportDetails;

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window);
SwapchainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
void destroySwapchainSupportDetails(SwapchainSupportDetails *details);
SwapchainDetails createSwapchain(
    VkDevice device, 
    const PhysicalDeviceDetails *physicalDevice, 
    VkSurfaceKHR surface, 
    GLFWwindow *window);
void destroySwapchain(VkDevice device, SwapchainDetails *swapchain);
void recreateSwapchain(
    VmaAllocator allocator,
    VkDevice device,
    const PhysicalDeviceDetails *physicalDevice,
    VkSurfaceKHR surface,
    GLFWwindow *window,
    VkRenderPass renderPass,
    VkSampleCountFlagBits samplingCount,
    SwapchainDetails *swapchain,
    DeviceImage *depthImage,
    DeviceImage *samplingImage,
    Framebuffers *framebuffers);