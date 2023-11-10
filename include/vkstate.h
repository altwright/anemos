#pragma once
#include <vulkan/vulkan.h>
#include "int.h"

#define VALIDATION_LAYERS_COUNT 1
extern const char* VALIDATION_LAYERS[VALIDATION_LAYERS_COUNT];

#define DEVICE_EXTENSIONS_COUNT 1
extern const char* DEVICE_EXTENSIONS[DEVICE_EXTENSIONS_COUNT];

#define MAX_FRAMES_IN_FLIGHT 2

/**
 * @brief Must call destroySwapchainSupportDetails() after
 */
typedef struct SwapchainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatsCount;
    VkSurfaceFormatKHR *formats;//free
    uint32_t presentModesCount;
    VkPresentModeKHR *presentModes;//free
} SwapchainSupportDetails;

typedef struct QueueFamilyIndices{
    uint32_t queueFamilyCount;
    uint32_t graphicsQueue;
    uint32_t presentQueue;
} QueueFamilyIndices;

typedef struct SwapchainDetails{
    VkSwapchainKHR handle;
    uint32_t imagesCount;
    VkImage *images;//free
    VkImageView *imageViews;//free
    VkFormat format;
    VkExtent2D extent;
} SwapchainDetails;

typedef struct PipelineDetails{
    VkPipeline handle;
    VkPipelineLayout layout;
} PipelineDetails;

typedef struct Framebuffers{
    size_t count;
    VkFramebuffer *handles;//free
} Framebuffers;

typedef struct Synchronisers{
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlight;
} Synchronisers;

typedef struct FrameControllers{
    VkCommandBuffer commandBuffer;
    Synchronisers synchronisers;
} FrameControllers;

typedef struct Buffer{
    VkBuffer handle;
    VkDeviceSize size;
    VkDeviceMemory memory;
    VkDeviceSize physicalSize;
} Buffer;

typedef struct PhysicalDeviceDetails{
    VkPhysicalDevice handle;
    VkPhysicalDeviceMemoryProperties memProperties;
} PhysicalDeviceDetails;

typedef struct DescriptorSet{
    VkDescriptorSet handle;
    Buffer buffer;
    void* mappedBuffer;
} DescriptorSet;

typedef struct Descriptors{
    DescriptorSet *sets;//free
    size_t setsCount;
    VkDescriptorSetLayout layout;
} Descriptors;

typedef struct Image{
    VkImage handle;
    VkDeviceMemory memory;
    u32 width;
    u32 height;
    u32 channels;
} Image;

typedef struct VkState{
    VkInstance instance;
    VkSurfaceKHR surface;
    PhysicalDeviceDetails physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    SwapchainDetails swapchain;
    VkRenderPass renderPass;
    VkDescriptorPool descriptorPool;
    Descriptors descriptors;
    PipelineDetails graphicsPipeline;
    Framebuffers framebuffers;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transferCommandPool;
    FrameControllers *frameContollers;//free
    Image texture;
} VkState;
