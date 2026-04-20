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

typedef struct ShadercInfo {
  shaderc_compiler_t compiler;
  shaderc_compile_options_t options;
} ShadercInfo;

mb_StringView read_glsl_and_compile_to_spv(mb_Arena *arena, ShadercInfo *sc, mb_StringView file_name, shaderc_shader_kind kind) {
  mb_File file = mb_open_file(file_name, mb_FileOpenMode_Text);
  assert(file);

  mb_TempArena temp = mb_begin_temp_arena(mb_get_scratch_arena(arena));
  mb_StringView shader_str = mb_file_read_bytes(temp.arena, file);
  mb_StringView file_name_without_dir = mb_str_split_from_right_till(&file_name, '/');

  shaderc_compilation_result_t sc_result = shaderc_compile_into_spv(sc->compiler, shader_str.data, shader_str.count, kind, mb_str_to_cstr(temp.arena, file_name_without_dir), "main", sc->options);

  mb_StringView str_view = {0};
  if(shaderc_result_get_num_errors(sc_result) == 0) {
    str_view.count = shaderc_result_get_length(sc_result),
    str_view.data = mb_arena_push(arena, char, .count = str_view.count);
    MemoryCopy(str_view.data, shaderc_result_get_bytes(sc_result), str_view.count);
  } else {
    // @TODO: let someone outside handle it instead.
    // For now it is fine to just print it out.
    fprintf(stdout, "Error compiling shader: %s\n", shaderc_result_get_error_message(sc_result));
  }

  shaderc_result_release(sc_result);
  mb_end_temp_arena(&temp);
  mb_close_file(file);
  return str_view;
}


int main() {
#if 0
  return window_application();
#else
  // compile spv
  ShadercInfo sc = { .compiler = shaderc_compiler_initialize(), .options = shaderc_compile_options_initialize() };
  shaderc_compile_options_set_source_language(sc.options, shaderc_source_language_glsl);
  shaderc_compile_options_set_optimization_level(sc.options, shaderc_optimization_level_performance);
  shaderc_compile_options_set_generate_debug_info(sc.options);


  mb_TempArena temp = mb_begin_temp_arena(0);
  mb_StringView spv_vert_shader = read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.vert"), shaderc_glsl_vertex_shader);
  mb_StringView spv_frag_shader = read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.frag"), shaderc_glsl_fragment_shader);
  if(spv_vert_shader.count) {
    mb_File spv_vert_file = mb_open_file(mb_str_from_cstr("data/shaders/color.vert.spv"), mb_FileOpenMode_Write);
    mb_file_write_bytes(spv_vert_file, spv_vert_shader);
    mb_close_file(spv_vert_file);
  }

  if(spv_frag_shader.count) {
    mb_File spv_frag_file = mb_open_file(mb_str_from_cstr("data/shaders/color.frag.spv"), mb_FileOpenMode_Write);
    mb_file_write_bytes(spv_frag_file, spv_frag_shader);
    mb_close_file(spv_frag_file);
  }

  mb_end_temp_arena(&temp);
  shaderc_compile_options_release(sc.options);
  shaderc_compiler_release(sc.compiler);
#endif
}

// build c files.
#include "base/arena.c"
#include "base/file.c"
#include "base/str.c"
#include "base/os.c"
#include "render/render.c"

