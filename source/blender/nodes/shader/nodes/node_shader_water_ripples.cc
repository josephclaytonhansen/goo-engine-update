// C++ Node Declaration
#include "node_shader_util.hh"

namespace blender::nodes::node_shader_water_ripples_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
    // Essential inputs
    b.add_input<decl::Vector>("Vector").default_value({0.0f, 0.0f, 0.0f});
    b.add_input<decl::Vector>("Normal").default_value({0.0f, 0.0f, 1.0f});
    b.add_input<decl::Float>("Time").default_value(0.0f);

    // Mode and main controls (now using enum)
    b.add_input<decl::Int>("Mode")
        .default_value(NODE_WATER_RIPPLES_DROPS)
        .min(NODE_WATER_RIPPLES_DROPS)
        .max(NODE_WATER_RIPPLES_RIPPLES)
        .description("0 = Drops, 1 = Ripples");
    b.add_input<decl::Float>("Scale").default_value(10.0f).min(1.0f).max(50.0f);
    b.add_input<decl::Float>("Intensity").default_value(1.0f).min(0.0f).max(5.0f);
    b.add_input<decl::Float>("Speed").default_value(1.0f).min(0.0f).max(5.0f);
    b.add_input<decl::Float>("Detail").default_value(0.5f).min(0.0f).max(1.0f);
    b.add_input<decl::Float>("Bias").default_value(0.6f).min(0.1f).max(0.9f);

    // Outputs
    b.add_output<decl::Vector>("Normal");
    b.add_output<decl::Float>("Mask");
}

static int gpu_shader_water_ripples(GPUMaterial *mat,
                                   bNode *node,
                                   bNodeExecData * /*execdata*/,
                                   GPUNodeStack *in,
                                   GPUNodeStack *out)
{
    return GPU_stack_link(mat, node, "node_water_ripples", in, out);
}

} // namespace blender::nodes::node_shader_water_ripples_cc

void register_node_type_sh_water_ripples(void)
{
    namespace file_ns = blender::nodes::node_shader_water_ripples_cc;
   
    static bNodeType ntype;
    sh_node_type_base(&ntype, SH_NODE_WATER_RIPPLES, "Water Ripples", NODE_CLASS_TEXTURE);
    ntype.declare = file_ns::node_declare;
    ntype.gpu_fn = file_ns::gpu_shader_water_ripples;
   
    nodeRegisterType(&ntype);
}