#include "vkdestroy.h"
#include <stdlib.h>

void destroyVkState(VkState *vkstate){
    vkDestroyPipeline(vkstate->logicalDevice, vkstate->pipeline.handle, NULL);
    vkDestroyPipelineLayout(vkstate->logicalDevice, vkstate->pipeline.layout, NULL);

    vkDestroyRenderPass(vkstate->logicalDevice, vkstate->renderPass, NULL);

    for (size_t i = 0; i < vkstate->swapchain.imagesCount; i++)
        vkDestroyImageView(vkstate->logicalDevice, vkstate->swapchain.imageViews[i], NULL);
    free(vkstate->swapchain.imageViews);
    free(vkstate->swapchain.images);

    vkDestroySwapchainKHR(vkstate->logicalDevice, vkstate->swapchain.handle, NULL);

    vkDestroyDevice(vkstate->logicalDevice, NULL);

    vkDestroySurfaceKHR(vkstate->instance, vkstate->surface, NULL);

    vkDestroyInstance(vkstate->instance, NULL);
}

void destroySwapchainSupportDetails(SwapchainSupportDetails *details){
    if (details->formats)
        free(details->formats);
    if (details->presentModes)
        free(details->presentModes);
}