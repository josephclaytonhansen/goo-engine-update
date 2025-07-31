/* SPDX-FileCopyrightText: 2025 Goo Engine Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup shdnodes
 */

#include "DNA_texture_types.h"
#include "DNA_brush_types.h"
#include "DNA_material_types.h"

#include "BKE_main.hh"
#include "BKE_node.hh"
#include "BKE_paint.hh"

#include "BLI_color.hh"
#include "BLI_string.h"

#include "DEG_depsgraph.hh"

#include "ED_node.hh"

#include "NOD_multi_function.hh"

#include "UI_interface.hh"
#include "UI_resources.hh"

#include "WM_api.hh"

#include "node_shader_util.hh"
#include "../../intern/node_util.hh"
#include "node_util.hh"

namespace blender::nodes::node_shader_color_palette_cc {

/* Callback function for selecting active color */
static void palette_active_color_set(bContext *C, void *palette_v, void *color_index_v)
{
  Palette *palette = (Palette *)palette_v;
  int color_index = POINTER_AS_INT(color_index_v);
  
  palette->active_color = color_index;
  
  /* Trigger depsgraph update for palette */
  DEG_id_tag_update(&palette->id, ID_RECALC_PARAMETERS);
  
  /* Force material recompilation by tagging all materials using this palette */
  Main *bmain = CTX_data_main(C);
  LISTBASE_FOREACH(Material *, ma, &bmain->materials) {
    if (ma->nodetree) {
      LISTBASE_FOREACH(bNode *, node, &ma->nodetree->nodes) {
        if (node->type == SH_NODE_COLOR_PALETTE) {
          NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
          if (data && data->palette == palette) {
            /* Force material shader recompilation */
            DEG_id_tag_update(&ma->id, ID_RECALC_SHADING);
            DEG_id_tag_update(&ma->nodetree->id, ID_RECALC_NTREE_OUTPUT);
          }
        }
      }
    }
  }
  
  WM_event_add_notifier(C, NC_MATERIAL | ND_SHADING, nullptr);
}

static void sh_node_color_palette_declare(NodeDeclarationBuilder &b)
{
  b.is_function_node();
  b.add_output<decl::Color>("Color");
  b.add_output<decl::Float>("Alpha");
}

static void node_shader_init_color_palette(bNodeTree * /*ntree*/, bNode *node)
{
  NodeShaderColorPalette *data = MEM_cnew<NodeShaderColorPalette>(__func__);
  
  /* Node starts with no palette - user will assign one */
  data->palette = nullptr;
  
  node->storage = data;
}

static void node_shader_draw_color_palette(uiLayout *layout, bContext *C, PointerRNA *ptr)
{
  bNode *node = (bNode *)ptr->data;
  NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
  
  /* Show palette selector */
  uiItemR(layout, ptr, "palette", UI_ITEM_NONE, nullptr, ICON_COLOR);
  
  /* If we have a palette, show color grid */
  if (data->palette != nullptr) {
    Palette *palette = data->palette;
    if (BLI_listbase_count(&palette->colors) > 0) {
      
      /* Show active color info */
      uiLayout *row = uiLayoutRow(layout, true);
      uiItemL(row, "Active Color:", ICON_NONE);
      char active_label[32];
      snprintf(active_label, sizeof(active_label), "%d", palette->active_color + 1);
      uiItemL(row, active_label, ICON_RADIOBUT_ON);
      
      /* Create a grid layout for colors */
      uiLayout *grid = uiLayoutGridFlow(layout, true, 3, true, true, true);
      
      int color_index = 0;
      LISTBASE_FOREACH_INDEX(PaletteColor *, color, &palette->colors, color_index) {
        if (color_index >= 9) break; /* Limit to 3x3 grid */
        
        uiLayout *col = uiLayoutColumn(grid, true);
        
        /* Create color display button (non-editable in node) */
        uiBlock *block = uiLayoutGetBlock(col);
        uiBut *color_but = uiDefButF(block, UI_BTYPE_COLOR, 0, "",
                                    0, 0, UI_UNIT_X * 2.0f, UI_UNIT_Y * 1.0f,
                                    color->rgb, 0.0f, 1.0f, "Color preview - Edit in Properties panel");
        
        /* Disable color editing in node - use Properties panel instead */
        UI_but_disable(color_but, "Edit in Properties panel");
        
        /* Add selection button below each color */
        char select_label[16];
        if (palette->active_color == color_index) {
          snprintf(select_label, sizeof(select_label), "● %d", color_index + 1);
        } else {
          snprintf(select_label, sizeof(select_label), "%d", color_index + 1);
        }
        
        uiLayout *select_row = uiLayoutRow(col, true);
        if (uiLayoutGetBlock(select_row)) {
          uiBlock *select_block = uiLayoutGetBlock(select_row);
          uiBut *select_but = uiDefBut(select_block, UI_BTYPE_BUT, 0, select_label,
                                       0, 0, UI_UNIT_X * 2.0f, UI_UNIT_Y * 0.6f,
                                       NULL, 0.0f, 0.0f, "Set as active color");
          
          /* Set button callback to change active color */
          UI_but_func_set(select_but, palette_active_color_set, palette, POINTER_FROM_INT(color_index));
        }
      }
    }
    else {
      uiItemL(layout, "Palette is empty", ICON_INFO);
    }
  }
  else {
    uiItemL(layout, "No palette selected", ICON_INFO);
  }
}

static int node_shader_gpu_color_palette(GPUMaterial *mat,
                                          bNode *node,
                                          bNodeExecData * /*execdata*/,
                                          GPUNodeStack *in,
                                          GPUNodeStack *out)
{
  NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
  
  if (data->palette != nullptr) {
    Palette *palette = data->palette;
    PaletteColor *active_color = (PaletteColor *)BLI_findlink(&palette->colors, palette->active_color);
    
    if (active_color != nullptr) {
      /* Output the active color from the palette */
      float color_with_alpha[4] = {active_color->rgb[0], active_color->rgb[1], active_color->rgb[2], 1.0f};
      return GPU_stack_link(mat, node, "node_color_palette_active", in, out, 
                            GPU_constant(color_with_alpha));
    }
    else {
      /* No active color - output white */
      float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
      return GPU_stack_link(mat, node, "node_color_palette_active", in, out, 
                            GPU_constant(white));
    }
  }
  else {
    /* No palette - output white */
    float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    return GPU_stack_link(mat, node, "node_color_palette_active", in, out, 
                          GPU_constant(white));
  }
}

static void node_shader_update_color_palette(bNodeTree *ntree, bNode *node)
{
  /* Standard node update */
}

}  // namespace blender::nodes::node_shader_color_palette_cc

/* node type definition */
void register_node_type_sh_color_palette(void)
{
  namespace file_ns = blender::nodes::node_shader_color_palette_cc;

  static bNodeType ntype;

  sh_node_type_base(&ntype, SH_NODE_COLOR_PALETTE, "Color Palette", NODE_CLASS_INPUT);
  ntype.declare = file_ns::sh_node_color_palette_declare;
  node_type_storage(
      &ntype, "NodeShaderColorPalette", node_free_standard_storage, node_copy_standard_storage);
  ntype.gpu_fn = file_ns::node_shader_gpu_color_palette;
  ntype.initfunc = file_ns::node_shader_init_color_palette;
  ntype.updatefunc = file_ns::node_shader_update_color_palette;
  ntype.draw_buttons = file_ns::node_shader_draw_color_palette;

  nodeRegisterType(&ntype);
}
