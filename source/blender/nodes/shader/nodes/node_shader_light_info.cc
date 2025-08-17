/* SPDX-FileCopyrightText: 2025 Goo Engine Authors
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

static void node_shader_init_light_info(bNodeTree * /*ntree*/, bNode *node)
{
  /* Node starts with no light object selected */
  node->id = nullptr;
}

/* Optional: Validation function to only allow light objects */
static bool node_light_info_id_poll(PointerRNA * /*ptr*/, PointerRNA *value)
{
  return value->data && ((Object *)value->data)->type == OB_LAMP;
}

static void node_shader_draw_light_info(uiLayout *layout, bContext *C, PointerRNA *ptr)
{
  /* Use Blender's built-in ID template for object selection */
  /* This will automatically bind to the node's "id" field */
  uiTemplateID(layout, C, ptr, "id", "object.add", nullptr, nullptr, 
               0, ICON_LIGHT, nullptr);
}

static int node_shader_gpu_light_info(GPUMaterial *mat,
                                      bNode *node,
                                      bNodeExecData * /*execdata*/,
                                      GPUNodeStack *in,
                                      GPUNodeStack *out)
{
  /* Always use safe defaults to avoid crashes during material compilation */
  float light_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float light_power = 1.0f;
  float perceptual_power = 1.0f;

  /* Use the node's id field which is properly serialized */
  if (node->id && GS(node->id->name) == ID_OB) {
    Object *light_ob = (Object *)node->id;
    
    if (light_ob->type == OB_LAMP && light_ob->data) {
      Light *light = (Light *)light_ob->data;
      
      if (light) {
        /* Safely get light properties */
        light_color[0] = light->r;
        light_color[1] = light->g;
        light_color[2] = light->b;
        light_color[3] = 1.0f;
        
        light_power = light->energy;
        
        /* Calculate perceptual power */
        perceptual_power = light_power;
      }
    }
  }

  /* Return shader call with color, power, and perceptual power */
  return GPU_stack_link(mat, node, "node_light_info", in, out,
                        GPU_constant(light_color),         /* Light Color */
                        GPU_constant(&light_power),        /* Light Power */
                        GPU_constant(&perceptual_power));  /* Perceptual Power */
}

}  // namespace blender::nodes::node_shader_light_info_cc

/* node type definition */
void register_node_type_sh_light_info(void)
{
  namespace file_ns = blender::nodes::node_shader_light_info_cc;
  
  static bNodeType ntype;
  sh_node_type_base(&ntype, SH_NODE_LIGHT_INFO, "Light Info", NODE_CLASS_INPUT);
  ntype.declare = file_ns::sh_node_light_info_declare;
  
  /* Set up the node to use ID datablock linking */
  ntype.idname = "ShaderNodeLightInfo";
  ntype.flag |= NODE_BACKGROUND;  // Optional: if you want it in background category
  
  /* Register functions */
  ntype.initfunc = file_ns::node_shader_init_light_info;
  ntype.draw_buttons = file_ns::node_shader_draw_light_info;
  ntype.gpu_fn = file_ns::node_shader_gpu_light_info;
  
  /* Enable ID linking for this node type */
  ntype.type = SH_NODE_LIGHT_INFO;
  
  nodeRegisterType(&ntype);
}