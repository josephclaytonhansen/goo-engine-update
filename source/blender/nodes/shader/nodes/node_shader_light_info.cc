/* SPDX-FileCopyrightText: 2025 Fruitbat Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup shdnodes
 */

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
#include "../../intern/node_util.hh"

namespace blender::nodes::node_shader_light_info_cc {

static void sh_node_light_info_declare(NodeDeclarationBuilder &b)
{
  b.add_output<decl::Color>("Light Color");
  b.add_output<decl::Float>("Light Power");
  b.add_output<decl::Float>("Perceptual Power");
  // b.add_output<decl::Float>("Light Distance"); // Temporarily removed - causes crash
}

static void node_shader_draw_light_info(uiLayout *layout, bContext * /*C*/, PointerRNA *ptr)
{
  /* Show light object selector */
  uiItemR(layout, ptr, "light_object", UI_ITEM_R_SPLIT_EMPTY_NAME, "Light", ICON_LIGHT);
}

static int node_shader_gpu_light_info(GPUMaterial *mat,
                                      bNode *node,
                                      bNodeExecData * /*execdata*/,
                                      GPUNodeStack *in,
                                      GPUNodeStack *out)
{
  Object *ob = (Object *)node->id;

  /* Always use safe defaults to avoid crashes during material compilation */
  float light_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float light_power = 1.0f;
  
  /* Only try to access object data if everything is valid */
  if (ob && ob != nullptr && 
      ob->type == OB_LAMP && 
      ob->data != nullptr) {
    
    Object *light_ob = ob;
    Light *light = (Light *)light_ob->data;
    
    /* Safely get light properties */
    light_color[0] = light->r;
    light_color[1] = light->g; 
    light_color[2] = light->b;
    light_color[3] = 1.0f;
    
    light_power = light->energy;
  }
  
  /* Return shader call with just color and power - no distance */
  return GPU_stack_link(mat, node, "node_light_info_simple", in, out,
                        GPU_constant(light_color),      /* Light Color */
                        GPU_constant(&light_power));    /* Light Power */
}

}  // namespace blender::nodes::node_shader_light_info_cc

/* node type definition */
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
