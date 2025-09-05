/* SPDX-FileCopyrightText: 2005 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "node_shader_util.hh"

#include "BLI_hash.h"

namespace blender::nodes::node_shader_input_aov_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::String>("Name", "Name");
  b.add_output<decl::Color>("Color", "Color");
  b.add_output<decl::Float>("Alpha", "Alpha");
}

static void node_shader_init_input_aov(bNodeTree * /*ntree*/, bNode *node)
{
  NodeShaderInputAOV *storage = MEM_cnew<NodeShaderInputAOV>("NodeShaderInputAOV");
  node->storage = storage;
}

static int node_shader_gpu_input_aov(GPUMaterial *mat,
                                     bNode *node,
                                     bNodeExecData * /*execdata*/,
                                     GPUNodeStack *in,
                                     GPUNodeStack *out)
{
  NodeShaderInputAOV *storage = static_cast<NodeShaderInputAOV *>(node->storage);
  
  const char *name = storage->name;
  if (name[0] == '\0') {
    name = "AOV";
  }

  uint hash = BLI_hash_string(name);
  float hash_float = *reinterpret_cast<float *>(&hash);
  GPUNodeLink *hash_link = GPU_constant(&hash_float);
  
  return GPU_stack_link(mat, node, "node_input_aov", in, out, hash_link);
}

}  // namespace blender::nodes::node_shader_input_aov_cc

void register_node_type_sh_input_aov()
{
  namespace file_ns = blender::nodes::node_shader_input_aov_cc;

  static bNodeType ntype;

  sh_node_type_base(&ntype, SH_NODE_INPUT_AOV, "AOV Input", NODE_CLASS_INPUT);
  ntype.declare = file_ns::node_declare;
  ntype.initfunc = file_ns::node_shader_init_input_aov;
  ntype.gpu_fn = file_ns::node_shader_gpu_input_aov;

  nodeRegisterType(&ntype);
}
