#include "node_util.hh"
#include "UI_interface.hh"
#include "UI_resources.hh"
#include "node_shader_util.hh"

namespace blender::nodes::node_shader_texture_gabor_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
    /* Inputs */
    b.add_input<decl::Vector>("Vector").default_value({0.0f, 0.0f, 0.0f}).implicit_field(implicit_field_inputs::position);
    b.add_input<decl::Float>("Scale").default_value(5.0f).min(0.0f).max(1000.0f).description("Overall scale of the noise pattern");
    b.add_input<decl::Float>("Frequency").default_value(2.0f).min(0.1f).max(20.0f).description("Frequency of the Gabor kernels");
    b.add_input<decl::Float>("Anisotropy").default_value(1.0f).min(0.0f).max(1.0f).description("0 = isotropic, 1 = anisotropic");
    b.add_input<decl::Float>("Orientation").default_value(0.0f).min(0.0f).max(1.0f).description("Direction of anisotropic noise (0-1 = 0-360°)");
    b.add_input<decl::Float>("Intensity").default_value(1.0f).min(0.0f).max(2.0f).description("Contrast of the noise");
    b.add_input<decl::Float>("Jittering").default_value(1.0f).min(0.0f).max(1.0f).description("0 = regular grid, 1 = jittered placement");
    b.add_input<decl::Float>("Density").default_value(1.0f).min(0.1f).max(4.0f).description("Number of splats per cell");
    
    /* Outputs */
    b.add_output<decl::Float>("Fac").no_muted_links();
    b.add_output<decl::Color>("Color");
}

static int gpu_shader_texture_gabor(GPUMaterial *mat,
                                   bNode *node,
                                   bNodeExecData * /*execdata*/,
                                   GPUNodeStack *in,
                                   GPUNodeStack *out)
{
    node_shader_gpu_default_tex_coord(mat, node, &in[0].link);
    node_shader_gpu_tex_mapping(mat, node, in, out);
    
    return GPU_stack_link(mat, node, "node_texture_gabor", in, out);
}

} // namespace blender::nodes::node_shader_texture_gabor_cc

void register_node_type_sh_tex_gabor(void)
{
    namespace file_ns = blender::nodes::node_shader_texture_gabor_cc;
   
    static bNodeType ntype;
    sh_node_type_base(&ntype, SH_NODE_TEX_GABOR, "Gabor Texture", NODE_CLASS_TEXTURE);
    ntype.declare = file_ns::node_declare;
    ntype.gpu_fn = file_ns::gpu_shader_texture_gabor;
    
    node_type_storage(&ntype, "", nullptr, nullptr);

    nodeRegisterType(&ntype);
}