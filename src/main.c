#include "base/base.h"
#include "base/arena.h"
#include "base/file.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <stdint.h>
#include "render/render.h"
#include <shaderc/shaderc.h>

static int window_application() {
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

int main() {
#if 0
  return window_application();
#else
  shaderc_compiler_t sc_compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t sc_options = shaderc_compile_options_initialize();
  shaderc_compile_options_set_source_language(sc_options, shaderc_source_language_glsl);
  shaderc_compile_options_set_optimization_level(sc_options, shaderc_optimization_level_performance);
  shaderc_compile_options_set_generate_debug_info(sc_options);

  mb_File vert_file = mb_open_file(mb_str_from_cstr("data/shaders/color.vert"), 0);
  mb_File frag_file = mb_open_file(mb_str_from_cstr("data/shaders/color.frag"), 0);
  assert(vert_file);
  assert(frag_file);

  mb_TempArena temp = mb_begin_temp_arena(0);
  mb_StringView vert_shader = mb_file_read_bytes(temp.arena, vert_file);
  mb_StringView frag_shader = mb_file_read_bytes(temp.arena, frag_file);

  shaderc_compilation_result_t sc_vert_result = shaderc_compile_into_spv(sc_compiler, vert_shader.data, vert_shader.count, shaderc_glsl_vertex_shader, "color.vert", "main", sc_options);
  shaderc_compilation_result_t sc_frag_result = shaderc_compile_into_spv(sc_compiler, frag_shader.data, frag_shader.count, shaderc_glsl_fragment_shader, "color.frag", "main", sc_options);

  if(shaderc_result_get_num_errors(sc_vert_result) == 0) {
    size_t sc_vert_length = shaderc_result_get_length(sc_vert_result);
    // const char *sc_vert_result_bytes = shaderc_result_get_bytes(sc_vert_result);
    fprintf(stdout, "Size of vertex shader: %zu\n", sc_vert_length);
    // @bookmark
  } else {
    fprintf(stdout, "Error compiling vert shader: %s\n", shaderc_result_get_error_message(sc_vert_result));
  }

  if(shaderc_result_get_num_errors(sc_frag_result) == 0) {
    size_t sc_frag_length = shaderc_result_get_length(sc_frag_result);
    // const char *sc_frag_result_bytes = shaderc_result_get_bytes(sc_frag_result);
    fprintf(stdout, "Size of fragment shader: %zu\n", sc_frag_length);
    // @bookmark
  } else {
    fprintf(stdout, "Error compiling frag shader: %s\n", shaderc_result_get_error_message(sc_frag_result));
  }

  shaderc_result_release(sc_vert_result);
  shaderc_result_release(sc_frag_result);
  mb_end_temp_arena(&temp);
  shaderc_compile_options_release(sc_options);
  shaderc_compiler_release(sc_compiler);
#endif
}

// build c files.
#include "base/arena.c"
#include "base/file.c"
#include "base/str.c"
#include "base/os.c"
#include "render/render.c"

