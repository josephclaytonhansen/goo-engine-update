#include "DNA_light_types.h"
#include "DNA_object_types.h"

#include "BKE_context.hh"
#include "BKE_node.hh"
#include "BKE_object.hh"

#include "GPU_material.hh"

#include "BLI_math_vector.h"
#include "BLI_math_matrix.h"
#include "BLI_math_rotation.h"

#include "DEG_depsgraph_query.hh"

#include "UI_interface.hh"
#include "UI_resources.hh"

#include "node_shader_util.hh"

namespace blender::nodes::node_shader_light_info_cc {

static void sh_node_light_info_declare(NodeDeclarationBuilder &b)
{
  b.add_output<decl::Color>("Light Color");
  b.add_output<decl::Float>("Light Power");
  b.add_output<decl::Float>("Perceptual Power");
}

static void node_shader_draw_light_info(uiLayout *layout, bContext * /*C*/, PointerRNA *ptr)
{
  /* Show light object selector */
  uiItemR(layout, ptr, "light_object", UI_ITEM_R_SPLIT_EMPTY_NAME, "Light", ICON_LIGHT);
}

/* This is the function called during GPU node graph building (before compilation). */
static int node_shader_gpu_light_info(GPUMaterial *mat,
                                      bNode *node,
                                      bNodeExecData * /*execdata*/,
                                      GPUNodeStack *in,
                                      GPUNodeStack *out)
{
  /* Access light properties and create uniforms that point directly to the Light's memory.
   * This allows animated light properties to update automatically without recompiling the shader. */
  Object *ob = (Object *)node->id;
  
  /* Always use safe defaults. */
  static float default_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  static float default_power = 1.0f;
  static float default_perceptual_power = 1.0f;
  
  GPUNodeLink *power_link;
  GPUNodeLink *perceptual_power_link;
  
  /* Try to access light properties if object is valid. */
  if (ob && ob->type == OB_LAMP && ob->data != nullptr) {
    Light *light = (Light *)ob->data;
    
    /* For animated light properties to work, we need to point the uniforms directly
     * to the Light's memory. Since r, g, b are consecutive floats in the Light struct,
     * we can create individual uniforms for each component and combine them. */
    
    /* Mark the material as needing per-object info updates.
     * This ensures that uniforms are re-uploaded each frame during animation playback. */
    GPU_material_flag_set(mat, GPU_MATFLAG_OBJECT_INFO);
    
    /* Create uniforms pointing directly to each RGB component */
    GPUNodeLink *r_link = GPU_uniform(&light->r);
    GPUNodeLink *g_link = GPU_uniform(&light->g);
    GPUNodeLink *b_link = GPU_uniform(&light->b);
    
    /* For power, point directly to the Light's energy field */
    power_link = GPU_uniform(&light->energy);
    perceptual_power_link = GPU_uniform(&light->energy);
    
    /* Use combine_rgb to build the final vec4 color from the three uniforms.
     * We can't pass the result of GPU_link to GPU_stack_link, so we use 
     * GPU_stack_link with combine_rgb directly. */
    return GPU_stack_link(mat, node, "node_light_info", in, out,
                          r_link,                    /* Red */
                          g_link,                    /* Green */
                          b_link,                    /* Blue */
                          power_link,                /* Light Power */
                          perceptual_power_link);    /* Perceptual Power */
  }
  else {
    /* Use default values */
    GPUNodeLink *color_link = GPU_uniform(default_color);
    power_link = GPU_uniform(&default_power);
    perceptual_power_link = GPU_uniform(&default_perceptual_power);
    
    /* Pass the default uniforms to the GPU stack. */
    return GPU_stack_link(mat, node, "node_light_info_simple", in, out,
                          color_link,                /* Light Color */
                          power_link,                /* Light Power */
                          perceptual_power_link);    /* Perceptual Power */
  }
}

}

void register_node_type_sh_light_info(void)
{
  namespace file_ns = blender::nodes::node_shader_light_info_cc;

  static blender::bke::bNodeType ntype;

  sh_node_type_base(&ntype, SH_NODE_LIGHT_INFO, "Light Info", NODE_CLASS_INPUT);
  ntype.declare = file_ns::sh_node_light_info_declare;
  ntype.draw_buttons = file_ns::node_shader_draw_light_info;
  ntype.gpu_fn = file_ns::node_shader_gpu_light_info;
  
  blender::bke::node_register_type(&ntype);
}