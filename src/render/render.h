#ifndef _RENDER_H_
#define _RENDER_H_

#include <SDL3/SDL.h>
#include <volk/volk.h>
#include "base/arena.h"

typedef struct WindowDynData {
  VkImage image;
  VkImageView view;
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

#endif //  _RENDER_H_
