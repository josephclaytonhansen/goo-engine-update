#include "node_shader_util.hh"

namespace blender::nodes::node_shader_sharpen_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Color>("Color").default_value({1.0f, 1.0f, 1.0f, 1.0f});
  b.add_input<decl::Vector>("Vector").implicit_field(implicit_field_inputs::position);
  b.add_input<decl::Float>("Gain").default_value(0.9f).min(0.0f).max(5.0f);
  b.add_input<decl::Vector>("Image Size").default_value({512.0f, 512.0f, 0.0f}).min(1.0f).max(8192.0f);
  b.add_output<decl::Color>("Color");
}

static int gpu_shader_sharpen(GPUMaterial *mat,
                              bNode *node,
                              bNodeExecData * /*execdata*/,
                              GPUNodeStack *in,
                              GPUNodeStack *out)
{
  return GPU_stack_link(mat, node, "node_sharpen", in, out);
}

} // namespace blender::nodes::node_shader_sharpen_cc

void register_node_type_sh_sharpen(void)
{
  namespace file_ns = blender::nodes::node_shader_sharpen_cc;
  
  static bNodeType ntype;
  sh_node_type_base(&ntype, SH_NODE_SHARPEN, "Sharpen", NODE_CLASS_OP_COLOR);
  ntype.declare = file_ns::node_declare;
  ntype.gpu_fn = file_ns::gpu_shader_sharpen;
  
  nodeRegisterType(&ntype);
}
