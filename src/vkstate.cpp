#include "vkstate.h"
#include <stdio.h>
#include <assert.h>
#include "window.h"
#include "config.h"
#include "vkinstance.h"
#include "vkswapchain.h"
#include "vkdevice.h"
#include "vkcommand.h"
#include "vkmemory.h"
#include "vkattachment.h"
#include "vkpipeline.h"

VulkanState initVulkanState(Window *window, const UserConfig *config)
{
    VulkanState vk = {};
    vk.instance = createInstance(config);
    vk.surface = createSurface(vk.instance, window->handle);
    vk.physicalDevice = selectPhysicalDevice(vk.instance, vk.surface);
    vk.device = createLogicalDevice(&vk.physicalDevice);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.graphicsQueue, 0, &vk.graphicsQueue);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.presentQueue, 0, &vk.presentQueue);
    vkGetDeviceQueue(vk.device, vk.physicalDevice.queueFamilyIndices.transferQueue, 0, &vk.transferQueue);
    vk.swapchain = createSwapchain(vk.device, &vk.physicalDevice, vk.surface, window->handle);
    vk.allocator = createAllocator(vk.device, vk.instance, vk.physicalDevice.handle);
    vk.depthImage = createDepthImage(
        vk.allocator, 
        vk.device, 
        vk.physicalDevice.handle, 
        vk.swapchain.extent, 
        vk.physicalDevice.maxSamplingCount);
    vk.samplingImage = createSamplingImage(
        vk.allocator,
        vk.device,
        vk.swapchain.format,
        vk.swapchain.extent,
        vk.physicalDevice.maxSamplingCount);
    vk.renderPass = createRenderPass(
        vk.device,
        vk.swapchain.format,
        vk.depthImage.format,
        vk.samplingImage.format,
        vk.physicalDevice.maxSamplingCount);
    vk.framebuffers = createFramebuffers(
        vk.device,
        vk.renderPass,
        &vk.swapchain,
        vk.depthImage.view,
        vk.samplingImage.view);
    vk.graphicsPipeline = createGraphicsPipeline(
        vk.device,
        vk.renderPass,
        vk.physicalDevice.maxSamplingCount);
    vk.graphicsCommandPool = createCommandPool(
        vk.device, 
        vk.physicalDevice.queueFamilyIndices.graphicsQueue, 
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    vk.transferCommandPool = createCommandPool(
        vk.device, 
        vk.physicalDevice.queueFamilyIndices.transferQueue, 
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

    return vk;
}

void destroyVulkanState(VulkanState *vk)
{
    vkDestroyCommandPool(vk->device, vk->transferCommandPool, NULL);
    vkDestroyCommandPool(vk->device, vk->graphicsCommandPool, NULL);

    vkDestroyPipeline(vk->device, vk->graphicsPipeline.handle, NULL);
    vkDestroyPipelineLayout(vk->device, vk->graphicsPipeline.layout, NULL);

    destroyFramebuffers(vk->device, &vk->framebuffers);

    vkDestroyRenderPass(vk->device, vk->renderPass, NULL);

    vkDestroyImageView(vk->device, vk->samplingImage.view, NULL);
    vmaDestroyImage(vk->allocator, vk->samplingImage.handle, vk->samplingImage.alloc);

    vkDestroyImageView(vk->device, vk->depthImage.view, NULL);
    vmaDestroyImage(vk->allocator, vk->depthImage.handle, vk->depthImage.alloc);

    destroySwapchain(vk->device, &vk->swapchain);
    vmaDestroyAllocator(vk->allocator);
    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
}