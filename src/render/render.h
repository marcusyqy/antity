#ifndef _RENDER_H_
#define _RENDER_H_

#include <SDL3/SDL.h>
#include <volk/volk.h>
#include "base/arena.h"
#include <vma/vk_mem_alloc.h>

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

extern VulkanResourceEngine vk_engine;

typedef struct WindowDynData {
  VkImage image;
  VkImageView view;
  VkFramebuffer fb;
  VkFence fence;
  VkSemaphore present_sem;
  VkSemaphore render_sem;
} WindowDynData;

typedef struct Window {
  SDL_Window    *sdl;
  mb_Arena      *arena;
  VkSurfaceKHR   surface;
  VkSwapchainKHR sc;

  WindowDynData  *d;
  uint32_t        d_count;
} Window;

void initialize_vulkan();
void cleanup_vulkan();

Window create_window(SDL_Window *sdl);
void   destroy_window(Window    *window);

// misc
const char *vk_result_to_string(VkResult result);

#define VK_EXPECT_SUCCESS(result) do {                                              \
  VkResult __result = (result);                                                     \
  if(__result != VK_SUCCESS) {                                                      \
    printf("Vk call `" #result "`: failed with %s", vk_result_to_string(__result)); \
    exit(1);                                                                        \
  }                                                                                 \
} while(0)

#endif //  _RENDER_H_
