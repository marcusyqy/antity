#include "render.h"
#include "base/arena.h"
#include <stdio.h>

#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#include <SDL3/SDL_vulkan.h>
#include <vma/vk_mem_alloc.h>


#define VK_RESULT_LIST                                     \
  X(VK_SUCCESS)                                            \
  X(VK_NOT_READY)                                          \
  X(VK_TIMEOUT)                                            \
  X(VK_EVENT_SET)                                          \
  X(VK_EVENT_RESET)                                        \
  X(VK_INCOMPLETE)                                         \
  X(VK_ERROR_OUT_OF_HOST_MEMORY)                           \
  X(VK_ERROR_OUT_OF_DEVICE_MEMORY)                         \
  X(VK_ERROR_INITIALIZATION_FAILED)                        \
  X(VK_ERROR_DEVICE_LOST)                                  \
  X(VK_ERROR_MEMORY_MAP_FAILED)                            \
  X(VK_ERROR_LAYER_NOT_PRESENT)                            \
  X(VK_ERROR_EXTENSION_NOT_PRESENT)                        \
  X(VK_ERROR_FEATURE_NOT_PRESENT)                          \
  X(VK_ERROR_INCOMPATIBLE_DRIVER)                          \
  X(VK_ERROR_TOO_MANY_OBJECTS)                             \
  X(VK_ERROR_FORMAT_NOT_SUPPORTED)                         \
  X(VK_ERROR_FRAGMENTED_POOL)                              \
  X(VK_ERROR_UNKNOWN)                                      \
  X(VK_ERROR_VALIDATION_FAILED)                            \
  X(VK_ERROR_OUT_OF_POOL_MEMORY)                           \
  X(VK_ERROR_INVALID_EXTERNAL_HANDLE)                      \
  X(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)               \
  X(VK_ERROR_FRAGMENTATION)                                \
  X(VK_PIPELINE_COMPILE_REQUIRED)                          \
  X(VK_ERROR_NOT_PERMITTED)                                \
  X(VK_ERROR_SURFACE_LOST_KHR)                             \
  X(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)                     \
  X(VK_SUBOPTIMAL_KHR)                                     \
  X(VK_ERROR_OUT_OF_DATE_KHR)                              \
  X(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)                     \
  X(VK_ERROR_INVALID_SHADER_NV)                            \
  X(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)                \
  X(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)       \
  X(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)    \
  X(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)       \
  X(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)        \
  X(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)          \
  X(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT) \
  X(VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT)                \
  X(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)          \
  X(VK_THREAD_IDLE_KHR)                                    \
  X(VK_THREAD_DONE_KHR)                                    \
  X(VK_OPERATION_DEFERRED_KHR)                             \
  X(VK_OPERATION_NOT_DEFERRED_KHR)                         \
  X(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR)             \
  X(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)                    \
  X(VK_INCOMPATIBLE_SHADER_BINARY_EXT)                     \
  X(VK_PIPELINE_BINARY_MISSING_KHR)                        \
  X(VK_ERROR_NOT_ENOUGH_SPACE_KHR)                         \
  X(VK_RESULT_MAX_ENUM)


static const char *vk_result_to_string(VkResult result) {
  switch(result) {
#define X(result) case result: return #result;
    VK_RESULT_LIST
#undef X
  }
  return "VkResultUnknown";
}

#define VK_EXPECT_SUCCESS(result) do {                                              \
  VkResult __result = (result);                                                     \
  if(__result != VK_SUCCESS) {                                                      \
    printf("Vk call `" #result "`: failed with %s", vk_result_to_string(__result)); \
    exit(1);                                                                        \
  }                                                                                 \
} while(0)

#define VK_MAX_QUEUES 3

typedef struct VulkanResourceEngine {
  VkAllocationCallbacks    *allocator;
  VkInstance               instance;
  VkDebugUtilsMessengerEXT debug;
  VkPhysicalDevice         physical_device;
  VkDevice                 device;
  VkQueue                  present_queue;
  VkQueue                  graphics_queue;
  VkQueue                  compute_queue;
  VmaAllocator             vma;
} VulkanResourceEngine;

static VulkanResourceEngine engine = {0};

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback,
    void *user_data) {
  (void)severity;
  (void)type;
  (void)user_data;
  // @TODO: add some cool logging?
  fprintf(stderr, "Validation layer: %s\n", callback->pMessage);
  return VK_FALSE;
}

static void vk_create_instance(void) {
  VkApplicationInfo app_info = {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext              = 0,
    .pApplicationName   = "Game",
    .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
    .pEngineName        = "Engine",
    .engineVersion      = VK_MAKE_VERSION(0, 0, 0),
    .apiVersion         = VK_API_VERSION_1_4,
  };

  uint32_t max_layer_count = 0;
  uint32_t extension_count = 0;
  vkEnumerateInstanceLayerProperties(&max_layer_count, 0);

  uint32_t layer_count = 0;
  const char *layer_names[1] = {"VK_LAYER_KHRONOS_validation"};

  mb_TempArena arena = mb_begin_temp_arena(mb_get_scratch_arena());
  VkLayerProperties *properties = mb_arena_push(arena.arena, VkLayerProperties, .count = max_layer_count);
  vkEnumerateInstanceLayerProperties(&max_layer_count, properties);


  for (uint32_t i = 0; i < max_layer_count; ++i) {
    fprintf(stdout, "VkInstance Layers %2u) %s : %s\n", i + 1, properties[i].layerName, properties[i].description);
    if (strcmp(properties[i].layerName, layer_names[0]) == 0) { layer_count = 1; }
  }

  // @NOTE: should we remove this. Feels abit error prone to end temp arena twice.
  mb_end_temp_arena(&arena);

  // For win32 surface and stuff.
  const char * const *sdl_instance_extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
  const char **instance_extensions = mb_arena_push(arena.arena, const char *, .count = extension_count + 1);
  MemoryCopy(instance_extensions, sdl_instance_extensions, sizeof(const char *) * extension_count);
  instance_extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  for(uint32_t i = 0; i < extension_count; ++i) { fprintf(stdout, "VkInstance Extension %u) %s\n", i + 1, instance_extensions[i]); }

  VkInstanceCreateInfo instance_create_info = {
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext                   = 0,
    .flags                   = 0,
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = layer_count,
    .ppEnabledLayerNames     = layer_names,
    .enabledExtensionCount   = extension_count,
    .ppEnabledExtensionNames = instance_extensions,
  };

  VK_EXPECT_SUCCESS(vkCreateInstance(&instance_create_info, engine.allocator, &engine.instance));

  mb_end_temp_arena(&arena);

}

static void vk_create_debug_utils_messenger(void) {
  VkDebugUtilsMessengerCreateInfoEXT create_info = {
    .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debug_utils_callback,
    .pUserData       = 0
  };
  VK_EXPECT_SUCCESS(vkCreateDebugUtilsMessengerEXT(engine.instance, &create_info, engine.allocator, &engine.debug));
}


VkSurfaceKHR vk_create_surface(SDL_Window *sdl) {
  VkSurfaceKHR surface = 0;
  bool result = SDL_Vulkan_CreateSurface(sdl, engine.instance, engine.allocator, &surface);
  assert(result);
  return surface;
}

static void vk_choose_physical_device() {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(engine.instance, &count, 0);

  mb_TempArena temp = mb_begin_temp_arena(mb_get_scratch_arena());

  VkPhysicalDevice *pd = mb_arena_push(temp.arena, VkPhysicalDevice, .count = count);
  vkEnumeratePhysicalDevices(engine.instance, &count, pd);

  for(uint32_t i = 0; i < count; ++i) {

    mb_TempArena scope_temp = mb_begin_temp_arena(temp.arena);
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pd[i], &queue_count, 0);

    VkQueueFamilyProperties *queue_props = mb_arena_push(temp.arena, VkQueueFamilyProperties, .count = queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(pd[i], &queue_count, queue_props);

    const uint8_t graphics_bit = 1 << 1;
    const uint8_t compute_bit  = 1 << 2;
    const uint8_t present_bit  = 1 << 3;
    const uint8_t ideal        = graphics_bit | compute_bit | present_bit;

    uint8_t queue_bit = 0;
    for(uint32_t j = 0; j < queue_count; ++j) {
      if((queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) queue_bit |= graphics_bit;
      if((queue_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)   queue_bit |= compute_bit;
      if(SDL_Vulkan_GetPresentationSupport(engine.instance, pd[i], j))                 queue_bit |= present_bit;
      if(ideal == queue_bit) break;
    }
    mb_end_temp_arena(&scope_temp);

    if(ideal == queue_bit) {
      engine.physical_device = pd[i];
      break;
    }
  }

  mb_end_temp_arena(&temp);
  assert(engine.physical_device);
}

static void vk_create_device(void) {
  mb_TempArena temp = mb_begin_temp_arena(mb_get_scratch_arena());

  uint32_t queue_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(engine.physical_device, &queue_count, 0);

  VkQueueFamilyProperties *queue_props = mb_arena_push(temp.arena, VkQueueFamilyProperties, .count = queue_count);
  vkGetPhysicalDeviceQueueFamilyProperties(engine.physical_device, &queue_count, queue_props);

  VkDeviceQueueCreateInfo queue_create_info[VK_MAX_QUEUES] = {0};
  uint32_t queue_create_info_count = 0;

  const float priority = 1.0f;

  const uint8_t graphics_bit = 1 << 1;
  const uint8_t compute_bit  = 1 << 2;
  const uint8_t present_bit  = 1 << 3;
  const uint8_t ideal        = graphics_bit | compute_bit | present_bit;

  uint8_t queue_bit = 0;
  for(uint32_t i = 0; i < queue_count; ++i) {
    uint8_t last_queue_bit = queue_bit;
    if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)  queue_bit |= graphics_bit;
    if((queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)    queue_bit |= compute_bit;
    if(SDL_Vulkan_GetPresentationSupport(engine.instance, engine.physical_device, i)) queue_bit |= present_bit;

    if(last_queue_bit != queue_bit) {
      queue_create_info[queue_create_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_create_info[queue_create_info_count].queueFamilyIndex = i;
      queue_create_info[queue_create_info_count].queueCount = 1; // queue_props[i].queueCount;
      queue_create_info[queue_create_info_count].pQueuePriorities = &priority;
      queue_create_info_count++;
    }

    if(queue_bit == ideal) break;
  }

  const char *enabled_extension[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  // nothing to enable yet.

  VkPhysicalDeviceVulkan12Features enabled_vk12_features = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .descriptorIndexing = VK_TRUE, .shaderSampledImageArrayNonUniformIndexing = VK_TRUE, .descriptorBindingVariableDescriptorCount = VK_TRUE, .runtimeDescriptorArray = VK_TRUE, .bufferDeviceAddress = VK_TRUE };
  VkPhysicalDeviceVulkan13Features enabled_vk13_features = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &enabled_vk12_features, .synchronization2 = VK_TRUE, .dynamicRendering = VK_TRUE };
  VkPhysicalDeviceFeatures enabled_features = { .samplerAnisotropy = VK_TRUE };

  VkDeviceCreateInfo create_info = {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                   = &enabled_vk13_features,
    .flags                   = 0,
    .queueCreateInfoCount    = queue_create_info_count,
    .pQueueCreateInfos       = queue_create_info,
    // @NOTE: enabledLayerCount is legacy and should not be used
    .enabledLayerCount       = 0,
    // @NOTE: ppEnabledLayerNames is legacy and should not be used
    .ppEnabledLayerNames     = 0,
    .enabledExtensionCount   = 1,
    .ppEnabledExtensionNames = enabled_extension,
    .pEnabledFeatures        = &enabled_features,
  };

  VK_EXPECT_SUCCESS(vkCreateDevice(engine.physical_device, &create_info, engine.allocator, &engine.device));
  assert(engine.device);

  volkLoadDevice(engine.device);


  // reset queue_bit for reuse
  queue_bit = 0;
  for(uint32_t i = 0; i < queue_count; ++i) {
    uint8_t last_queue_bit = queue_bit;
    b8 supports_graphics_queue = (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT;
    b8 supports_compute_queue = (queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT;
    b8 supports_present_queue = SDL_Vulkan_GetPresentationSupport(engine.instance, engine.physical_device, i);

    if(supports_graphics_queue) queue_bit |= graphics_bit;
    if(supports_compute_queue)  queue_bit |= compute_bit;
    if(supports_present_queue)  queue_bit |= present_bit;

    if(last_queue_bit != queue_bit) {
      VkQueue queue = 0;
      // @TODO: figure out whats happening here.
      vkGetDeviceQueue(engine.device, i, 0, &queue);
      assert(queue);

      if(supports_graphics_queue) engine.graphics_queue = queue;
      if(supports_compute_queue)  engine.compute_queue = queue;
      if(supports_present_queue)  engine.present_queue = queue;
    }

    if(queue_bit == ideal) break;
  }
  mb_end_temp_arena(&temp);
}

// static void vk_create_or_recreate_swapchain(Window *window) {
//   (void)window;
//
//   VkSurfaceCapabilitiesKHR surface_capabilities = {0};
//   VK_EXPECT_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine.physical_device, window->surface, &surface_capabilities));
//
//   VkSwapchainCreateInfoKHR create_info = {
//     .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
//     .pNext = 0,
//     .flags = 0,
//     .surface = window->surface,
//     .minImageCount = surface_capabilities.minImageCount,
//     VkFormat                         imageFormat;
//     VkColorSpaceKHR                  imageColorSpace;
//     VkExtent2D                       imageExtent;
//     uint32_t                         imageArrayLayers;
//     VkImageUsageFlags                imageUsage;
//     VkSharingMode                    imageSharingMode;
//     uint32_t                         queueFamilyIndexCount;
//     const uint32_t*                  pQueueFamilyIndices;
//     VkSurfaceTransformFlagBitsKHR    preTransform;
//     VkCompositeAlphaFlagBitsKHR      compositeAlpha;
//     VkPresentModeKHR                 presentMode;
//     VkBool32                         clipped;
//     .oldSwapchain = window->swapchain,
//   };
//
//   VK_EXPECT_SUCCESS(vkCreateSwapchainKHR(engine.device, &create_info, engine.allocator, &window->sc));
// }

void initialize_vulkan(void) {
  VK_EXPECT_SUCCESS(volkInitialize());
  vk_create_instance();
  volkLoadInstance(engine.instance);
  vk_create_debug_utils_messenger();
  vk_choose_physical_device();

  VkPhysicalDeviceProperties physical_device_properties = {0};
  vkGetPhysicalDeviceProperties(engine.physical_device, &physical_device_properties);
  fprintf(stdout, "Chosen physical device :%s\n", physical_device_properties.deviceName);
  vk_create_device();

  VmaVulkanFunctions vk_functions = {
    .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
    .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    .vkCreateImage = vkCreateImage
  };
  VmaAllocatorCreateInfo vma_create_info = {
    .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    .physicalDevice = engine.physical_device,
    .device = engine.device,
    .pVulkanFunctions = &vk_functions,
    .instance = engine.instance
  };
  VK_EXPECT_SUCCESS(vmaCreateAllocator(&vma_create_info, &engine.vma));
}

void cleanup_vulkan(void) {
  VK_EXPECT_SUCCESS(vkDeviceWaitIdle(engine.device));

  vkDestroyInstance(engine.instance, engine.allocator);
  vkDestroyDevice(engine.device, engine.allocator);
}

// @TODO: we can change this to just create window and then from there initialize vulkan if we have not.
Window create_window(SDL_Window *sdl) {
  // figure out when we should initialize (create_window right now cannot be called twice).
  Window window = {0};
  window.surface = vk_create_surface(sdl);
  // vk_create_or_recreate_swapchain(&window);
  return window;
}

void destroy_window(Window *window) {
  assert(window);
  SDL_Vulkan_DestroySurface(engine.instance, window->surface, engine.allocator);
}
