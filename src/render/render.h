#ifndef _RENDER_H_
#define _RENDER_H_

#include <SDL3/SDL.h>
#include <volk/volk.h>

typedef struct Window {
  VkSurfaceKHR surface;
  VkSwapchainKHR sc;
  SDL_Window *sdl;
} Window;

Window create_window(SDL_Window *sdl);
void destroy_window(Window *window);

#endif //  _RENDER_H_
