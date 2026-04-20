#include "base/base.h"
#include "base/arena.h"
#include "base/file.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <stdint.h>
#include "render/render.h"
#include "render/shader_compiler.h"

#include <volk/volk.h>

typedef struct RenderPipeline {
  VkPipeline handle;
  VkPipelineLayout layout;
} RenderPipeline;

static RenderPipeline create_default_render_pipeline(void) {
  RenderPipeline rp = {0};

  // @TODO: when we need to pass information to the shader.
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .setLayoutCount = 0,
    .pSetLayouts = 0,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges = 0,
  };
  VK_EXPECT_SUCCESS(vkCreatePipelineLayout(vk_engine.device, &pipeline_layout_create_info, vk_engine.allocator, &rp.layout));

  ShadercInfo sc = shader_compiler_create();
  // compile spv
  mb_TempArena temp = mb_begin_temp_arena(0);
  mb_StringView spv_vert_shader = shader_compiler_read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.vert"), shaderc_glsl_vertex_shader);
  mb_StringView spv_frag_shader = shader_compiler_read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.frag"), shaderc_glsl_fragment_shader);
  assert(spv_vert_shader.count && spv_frag_shader.count);


  VkShaderModule vert_shader_module = 0;
  VkShaderModuleCreateInfo vert_shader_ci = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .codeSize = spv_vert_shader.count,
    .pCode = (const uint32_t *)spv_vert_shader.data,
  };
  VK_EXPECT_SUCCESS(vkCreateShaderModule(vk_engine.device, &vert_shader_ci, vk_engine.allocator, &vert_shader_module));

  VkShaderModule frag_shader_module = 0;
  VkShaderModuleCreateInfo shader_ci = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .codeSize = spv_frag_shader.count,
    .pCode = (const uint32_t *)spv_frag_shader.data,
  };
  VK_EXPECT_SUCCESS(vkCreateShaderModule(vk_engine.device, &shader_ci, vk_engine.allocator, &frag_shader_module));

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader_module,
      .pName = "main",
    },
    (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader_module,
      .pName = "main",
    }
  };

  // this is now hardcoded inside the window as well.
  const VkFormat color_format = VK_FORMAT_B8G8R8A8_SRGB;
  VkPipelineRenderingCreateInfo rendering_ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .pNext = 0,
    .viewMask = 0,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
    .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .vertexBindingDescriptionCount = 0,
    .pVertexBindingDescriptions = 0,
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions = 0,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = 0,
  };

  VkPipelineViewportStateCreateInfo viewport_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .viewportCount = 1,
    .pViewports = 0, // 0 because of dynamic state.
    .scissorCount = 1,
    .pScissors = 0, // 0 because of dynamic state.
  };

  VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .depthClampEnable = 0,
    .rasterizerDiscardEnable = 0,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = 0,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = 0,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = 0,
    .alphaToOneEnable = 0,
  };

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .depthTestEnable = 0,
    .depthWriteEnable = 0,
    .depthCompareOp = VK_COMPARE_OP_NEVER,
    .depthBoundsTestEnable = 0,
    .stencilTestEnable = 0,
    .front = (VkStencilOpState){0},
    .back = (VkStencilOpState){0},
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 0.0f,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
    .blendEnable = 0,
    .srcColorBlendFactor = 0,
    .dstColorBlendFactor = 0,
    .colorBlendOp = 0,
    .srcAlphaBlendFactor = 0,
    .dstAlphaBlendFactor = 0,
    .alphaBlendOp = 0,
    .colorWriteMask = 0,

  };
  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .logicOpEnable = 0,
    .logicOp = VK_LOGIC_OP_CLEAR,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state,
    .blendConstants = {0.f, 0.f, 0.f, 0.f},
 };

  VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext = 0,
    .flags = 0,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };

  VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_ci,
    .flags = 0,
    .stageCount = ArrayCount(shader_stages),
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input_state_create_info,
    .pInputAssemblyState = &input_assembly_state_create_info,
    .pTessellationState = 0,
    .pViewportState = &viewport_state_create_info,
    .pRasterizationState = &rasterization_state_create_info,
    .pMultisampleState = &multisample_state_create_info,
    .pDepthStencilState = &depth_stencil_state_create_info,
    .pColorBlendState = &color_blend_state_create_info,
    .pDynamicState = &dynamic_state_create_info,
    .layout = rp.layout,
    .renderPass = 0,
    .subpass = 0,
    .basePipelineHandle = 0,
    .basePipelineIndex = 0
  };

  VK_EXPECT_SUCCESS(vkCreateGraphicsPipelines(vk_engine.device, 0, 1, &graphics_pipeline_create_info, vk_engine.allocator, &rp.handle));

  mb_end_temp_arena(&temp);
  shader_compiler_destroy(&sc);
  vkDestroyShaderModule(vk_engine.device, vert_shader_module, vk_engine.allocator);
  vkDestroyShaderModule(vk_engine.device, frag_shader_module, vk_engine.allocator);

  return rp;
}

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

  RenderPipeline rp = create_default_render_pipeline();
  (void)rp;

  SDL_Event event = {0};
  b8 running = 1;
  while(running) {
    while (SDL_PollEvent(&event)) {
      if(event.type == SDL_EVENT_QUIT) running = 0;
      if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window.sdl)) running = 0;
      if(event.type == SDL_EVENT_WINDOW_RESIZED && event.window.windowID == SDL_GetWindowID(window.sdl)) {
        window_width  = (uint32_t)event.window.data1;
        window_height = (uint32_t)event.window.data2;
      }
      if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_Q) running = 0;
    }

    SDL_GL_SwapWindow(window.sdl);
  }

  destroy_window(&window);
  vkDestroyPipeline(vk_engine.device, rp.handle, vk_engine.allocator);
  vkDestroyPipelineLayout(vk_engine.device, rp.layout, vk_engine.allocator);
  SDL_DestroyWindow(window.sdl);
  cleanup_vulkan();
  SDL_Quit();
  return 0;
}

int main() {
#if 1
  return window_application();
#else
  ShadercInfo sc = shader_compiler_create();
  // compile spv
  mb_TempArena temp = mb_begin_temp_arena(0);
  mb_StringView spv_vert_shader = shader_compiler_read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.vert"), shaderc_glsl_vertex_shader);
  mb_StringView spv_frag_shader = shader_compiler_read_glsl_and_compile_to_spv(temp.arena, &sc, mb_str_from_cstr("data/shaders/color.frag"), shaderc_glsl_fragment_shader);
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
  shader_compiler_destroy(&sc);
#endif
}

// build c files.
#include "base/arena.c"
#include "base/file.c"
#include "base/str.c"
#include "base/os.c"
#include "render/render.c"
#include "render/shader_compiler.c"

