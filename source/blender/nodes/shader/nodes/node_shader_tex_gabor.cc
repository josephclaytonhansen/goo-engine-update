#include "node_util.hh"
#include "UI_interface.hh"
#include "UI_resources.hh"
#include "node_shader_util.hh"
#include "BKE_texture.h"
#include "DNA_node_types.h"

namespace blender::nodes::node_shader_texture_gabor_cc {

static void node_shader_init_tex_gabor(bNodeTree * /*ntree*/, bNode *node)
{
    NodeTexGabor *tex = MEM_cnew<NodeTexGabor>("NodeTexGabor");
    BKE_texture_mapping_default(&tex->base.tex_mapping, TEXMAP_TYPE_POINT);
    BKE_texture_colormapping_default(&tex->base.color_mapping);
    tex->anisotropic = 1;  /* Default to anisotropic */
    tex->jittering = 1;    /* Default to jittered */
    node->storage = tex;
}

static void node_layout(uiLayout *layout, bContext * /*C*/, PointerRNA *ptr)
{
    uiLayout *col = uiLayoutColumn(layout, false);
    uiItemR(col, ptr, "anisotropic", UI_ITEM_R_SPLIT_EMPTY_NAME, nullptr, ICON_NONE);
    uiItemR(col, ptr, "jittering", UI_ITEM_R_SPLIT_EMPTY_NAME, nullptr, ICON_NONE);
}

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
    NodeTexGabor *tex = (NodeTexGabor *)node->storage;
    if (!tex) {
        return 0;
    }
    
    node_shader_gpu_default_tex_coord(mat, node, &in[0].link);
    node_shader_gpu_tex_mapping(mat, node, in, out);
    
    /* Pass the enum values as constants to the shader */
    float anisotropic = (float)tex->anisotropic;
    float jittering = (float)tex->jittering;
    
    return GPU_stack_link(mat, node, "node_texture_gabor", in, out, 
                         GPU_constant(&anisotropic), GPU_constant(&jittering));
}

} // namespace blender::nodes::node_shader_texture_gabor_cc

void register_node_type_sh_tex_gabor(void)
{
    namespace file_ns = blender::nodes::node_shader_texture_gabor_cc;
   
    static bNodeType ntype;
    sh_node_type_base(&ntype, SH_NODE_TEX_GABOR, "Gabor Texture", NODE_CLASS_TEXTURE);
    ntype.declare = file_ns::node_declare;
    ntype.draw_buttons = file_ns::node_layout;
    ntype.initfunc = file_ns::node_shader_init_tex_gabor;
    ntype.gpu_fn = file_ns::gpu_shader_texture_gabor;
    
    node_type_storage(&ntype, "NodeTexGabor", node_free_standard_storage, node_copy_standard_storage);

    nodeRegisterType(&ntype);
}