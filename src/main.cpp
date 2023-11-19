#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "config.h"
#include "vkstate.h"
#include "model.h"
#include "vkbuffer.h"
#include "vkswapchain.h"
#include "vkcommand.h"
#include "vkshader.h"
#include "input.h"
#include "controls.h"

int main(int, char**){
    UserConfig userConfig = {};
    if (!loadUserConfig(CONFIG_FILE, &userConfig)){
        fprintf(stderr, "Failed to load complete user config\n");
        exit(EXIT_FAILURE);
    }

    InputHandler inputHandler = createInputHandler();
    Window window = {};
    if (!initWindow(
        TITLE, 
        userConfig.window.width, 
        userConfig.window.height, 
        &inputHandler, 
        &window))
    {
        fprintf(stderr, "Failed to create GLFW window!\n");
        exit(EXIT_FAILURE);
    }

    CameraControlState controlState = {};
    inputHandler.ctx = &controlState;
    inputHandler.w = &cam_handleKeyW;
    inputHandler.s = &cam_handleKeyS;

    VulkanState vk = initVulkanState(&window, &userConfig);

    Model cube = loadModel("./models/cube.glb");

    size_t bytesCount = 0;
    unsigned char *mappedBuffer = (unsigned char*)vk.stagingBuffer.info.pMappedData;
    memcpy(mappedBuffer, cube.vertices, sizeof(Vertex)*cube.verticesCount);
    bytesCount += sizeof(Vertex)*cube.verticesCount;
    mappedBuffer += bytesCount;
    memcpy(mappedBuffer, cube.indices, sizeof(u16)*cube.indicesCount);
    bytesCount += sizeof(u16)*cube.indicesCount;

    copyToDeviceBuffer(
        bytesCount, 
        vk.stagingBuffer.handle, 0, 
        vk.deviceBuffer.handle, 0, 
        vk.device, 
        vk.transferCommandPool, 
        vk.transferQueue);

    VkCommandBuffer graphicsCmdBuffers[MAX_FRAMES_IN_FLIGHT] = {};
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        graphicsCmdBuffers[i] = createPrimaryCommandBuffer(vk.device, vk.graphicsCmdPools[i]);
    }
    u32 currentFrame = 0;
    while (!glfwWindowShouldClose(window.handle))
    {
        vkWaitForFences(vk.device, 1, &vk.frameSyncers[currentFrame].inFlight, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;//Will refer to a VkImage in our swapchain images array
        VkResult result = vkAcquireNextImageKHR(
            vk.device, 
            vk.swapchain.handle, 
            UINT64_MAX, 
            vk.frameSyncers[currentFrame].imageAvailable, 
            VK_NULL_HANDLE, 
            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain(
                vk.allocator,
                vk.device,
                &vk.physicalDevice,
                vk.surface,
                window.handle,
                vk.renderPass,
                vk.physicalDevice.maxSamplingCount,
                &vk.swapchain,
                &vk.depthImage,
                &vk.samplingImage,
                &vk.framebuffers);

            continue;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            printf("Failed to Acquire Next Swapchain Image: %d\n", result);
            exit(EXIT_FAILURE);
        }

        vkResetFences(vk.device, 1, &vk.frameSyncers[currentFrame].inFlight);

        vkResetCommandPool(vk.device, vk.graphicsCmdPools[currentFrame], 0);

        PushConstant pushConstant = updatePushConstant(vk.swapchain.extent);

        recordDrawCommand(
            graphicsCmdBuffers[currentFrame],
            vk.renderPass,
            vk.framebuffers.handles[imageIndex],
            vk.graphicsPipeline,
            vk.swapchain.extent,
            pushConstant,
            vk.deviceBuffer.handle,
            0,
            cube.verticesCount,
            sizeof(Vertex)*cube.verticesCount,
            cube.indicesCount
        );

        submitDrawCommand(
            vk.graphicsQueue,
            graphicsCmdBuffers[currentFrame],
            vk.frameSyncers[currentFrame].imageAvailable,
            vk.frameSyncers[currentFrame].renderFinished,
            vk.frameSyncers[currentFrame].inFlight
        );

        result = presentSwapchain(
            vk.presentQueue,
            vk.frameSyncers[currentFrame].renderFinished,
            vk.swapchain.handle,
            imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR || 
            result == VK_SUBOPTIMAL_KHR || 
            window.resizing)
        {
            recreateSwapchain(
                vk.allocator,
                vk.device,
                &vk.physicalDevice,
                vk.surface,
                window.handle,
                vk.renderPass,
                vk.physicalDevice.maxSamplingCount,
                &vk.swapchain,
                &vk.depthImage,
                &vk.samplingImage,
                &vk.framebuffers);

            window.resizing = false;
        }
        else if (result != VK_SUCCESS){
            printf("Failed to Present Swapchain Image\n");
            exit(EXIT_FAILURE);
        }
            
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    vkDeviceWaitIdle(vk.device);

    freeModel(&cube);
    destroyWindow(&window);
    destroyVulkanState(&vk);

    return 0;
}
