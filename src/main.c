#include "base/base.h"
#include "base/arena.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <stdint.h>
#include "render/render.h"

int main() {
  if(!SDL_Init(SDL_INIT_VIDEO)) fprintf(stderr, "Failed to initialize SDL3: %s\n", SDL_GetError());
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");
  initialize_vulkan();

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  fprintf(stdout, "Main scale %f\n", main_scale);

  SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY; // | SDL_WINDOW_MAXIMIZED; | SDL_WINDOW_RESIZABLE

  // @TODO: figure out this (probably good for high dpi stuff).
  // SDL_Rect rect = {};
  // SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &rect);

  uint32_t window_width = 1200;
  uint32_t window_height = 800;

  SDL_Window *sdl = SDL_CreateWindow("Game", window_width, window_height, window_flags);
  if(!sdl) {
    fprintf(stdout, "Failed to create SDL3 window: %s\n", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  Window window = create_window(sdl);
  window.w = window_width;
  window.h = window_height;

  SDL_Event event = {0};
  bool running = true;
  while(running) {
    while (SDL_PollEvent(&event)) {
      if(event.type == SDL_EVENT_QUIT) running = false;
      if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window.sdl)) running = false;
      if(event.type == SDL_EVENT_WINDOW_RESIZED && event.window.windowID == SDL_GetWindowID(window.sdl)) {
        window_width  = (uint32_t)event.window.data1;
        window_height = (uint32_t)event.window.data2;
      }

      if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_Q) running = false;
    }

    SDL_GL_SwapWindow(window.sdl);
  }

  destroy_window(&window);
  SDL_DestroyWindow(window.sdl);
  cleanup_vulkan();
  SDL_Quit();
  return 0;
}

// build c files.
#include "base/arena.c"
#include "base/os.c"
#include "render/render.c"
