#include "vkcommand.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include "vkstate.h"
#include "int.h"

void recordDrawCommand(
    VkCommandBuffer commandBuffer, 
    VkRenderPass renderPass, 
    VkFramebuffer framebuffer, 
    PipelineDetails graphicsPipeline,
    const SwapchainDetails *swapchainDetails,
    VkDescriptorSet descriptorSet,
    Buffer vertexBuffer,
    VkDeviceSize vertexBufferOffset,
    u32 vertexCount,
    Buffer indexBuffer,
    VkDeviceSize indexBufferOffset,
    u32 indexCount)
{

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo)){//Implicit reset of buffer
        printf("Failed to begin recording Command Buffer\n");
        exit(EXIT_FAILURE);
    }

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapchainDetails->extent;
    //The render area defines where shader loads and stores will take place. 
    //The pixels outside this region will have undefined values. It should 
    //match the size of the attachments for best performance.
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.handle);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.handle, &vertexBufferOffset);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.handle, indexBufferOffset, VK_INDEX_TYPE_UINT16);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapchainDetails->extent.width;
    viewport.height = swapchainDetails->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainDetails->extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        graphicsPipeline.layout, 
        0, 
        1, 
        &descriptorSet, 
        0, 
        NULL
    );

    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer)){
        printf("Failed to end recording of Command Buffer\n");
        exit(EXIT_FAILURE);
    }
}

void submitDrawCommand(
    VkQueue graphicsQueue, 
    VkCommandBuffer commandBuffer, 
    VkSemaphore waitSemaphore,
    VkSemaphore signalSemaphore,
    VkFence hostFence){

    VkSubmitInfo sumbitInfo{};
    sumbitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sumbitInfo.waitSemaphoreCount = 1;
    sumbitInfo.pWaitSemaphores = &waitSemaphore;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    sumbitInfo.pWaitDstStageMask = waitStages;
    sumbitInfo.commandBufferCount = 1;
    sumbitInfo.pCommandBuffers = &commandBuffer;
    sumbitInfo.signalSemaphoreCount = 1;
    sumbitInfo.pSignalSemaphores = &signalSemaphore;//Signalled once the command buffers have finished executing

    //hostFence signals to the host that it is safe to reset the command buffer,
    //which is an optional feature.
    if (vkQueueSubmit(graphicsQueue, 1, &sumbitInfo, hostFence)){
        printf("Submission of Command Buffer failed\n");
        exit(EXIT_FAILURE);
    }
}

VkResult presentSwapchain(
    VkQueue presentQueue, 
    VkSemaphore waitSemaphore,
    VkSwapchainKHR swapchain,
    uint32_t swapchainImageIndex)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &swapchainImageIndex;

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

VkCommandBuffer beginSingleTimeCommand(VkDevice device, VkCommandPool transientCommandPool)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transientCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = NULL;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)){
        fprintf(stderr, "Failed to allocated Single Use Command Buffer\n");
        exit(EXIT_FAILURE);
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void submitSingleTimeCommand(
    VkDevice device, 
    VkCommandPool transientCommandPool, 
    VkCommandBuffer commandBuffer, 
    VkQueue queue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE)){
        fprintf(stderr, "Failed to submit single time command to Queue\n");
        exit(EXIT_FAILURE);
    }

    if (vkQueueWaitIdle(queue)){
        fprintf(stderr, "Failure waiting for Queue to Idle\n");
        exit(EXIT_FAILURE);
    }

    vkFreeCommandBuffers(device, transientCommandPool, 1, &commandBuffer);
}