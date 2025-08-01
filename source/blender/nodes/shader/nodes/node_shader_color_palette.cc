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
#include "BKE_context.hh"
#include "BKE_lib_id.hh"
#include "BKE_global.hh"
#include "BKE_material.h"

#include "GPU_material.hh"

#include "BLI_color.hh"
#include "BLI_string.h"
#include "BLI_listbase.h"
#include "BLI_math_vector.h"
#include "BLI_math_base.h"

#include "MEM_guardedalloc.h"

#include "DEG_depsgraph.hh"

#include "ED_node.hh"

#include "NOD_multi_function.hh"

#include "UI_interface.hh"
#include "UI_resources.hh"

#include "WM_api.hh"

#include "RNA_access.hh"
#include "RNA_prototypes.h"

/* Need access to uiButColor for palette color buttons */
#include "../../../editors/interface/interface_intern.hh"

#include "node_shader_util.hh"
#include "../../intern/node_util.hh"
#include "node_util.hh"

namespace blender::nodes::node_shader_color_palette_cc {

/* Forward declarations */
static void sync_node_palette_from_source(bContext *C, bNode *node, NodeShaderColorPalette *data);
static Palette *create_node_palette_copy(Main *bmain, Palette *source, const char *node_name);
static void palette_color_changed_callback(bContext *C, void *node_v, void *unused);

/* Callback function for when palette colors change */
static void palette_color_changed_callback(bContext *C, void *node_v, void *unused)
{
  bNode *node = (bNode *)node_v;
  
  /* Force material shader recompilation - find the material this node belongs to */
  Main *bmain = CTX_data_main(C);
  LISTBASE_FOREACH(Material *, ma, &bmain->materials) {
    if (ma->nodetree) {
      LISTBASE_FOREACH(bNode *, check_node, &ma->nodetree->nodes) {
        if (check_node == node && check_node->type == SH_NODE_COLOR_PALETTE) {
          /* Force shader recompilation for this specific material */
          DEG_id_tag_update(&ma->id, ID_RECALC_SHADING);
          DEG_id_tag_update(&ma->nodetree->id, ID_RECALC_NTREE_OUTPUT);
          
          /* Also trigger a more aggressive update to ensure GPU shader recompilation */
          if (ma->gpumaterial.first) {
            GPU_material_free(&ma->gpumaterial);
          }
        }
      }
    }
  }
  
  /* Trigger comprehensive UI and viewport updates */
  WM_event_add_notifier(C, NC_MATERIAL | ND_SHADING, nullptr);
  WM_event_add_notifier(C, NC_MATERIAL | ND_SHADING_DRAW, nullptr);
  WM_event_add_notifier(C, NC_MATERIAL | ND_SHADING_LINKS, nullptr);
}

/* Create a copy of a palette for this specific node */
static Palette *create_node_palette_copy(Main *bmain, Palette *source, const char *node_name)
{
  if (!source) return nullptr;
  
  /* Create a new palette with a unique name based on the ORIGINAL palette name */
  char palette_name[256];
  const char *source_name = source->id.name + 2; /* Skip ID prefix */
  
  /* If source is already a node copy, extract the original name */
  if (strncmp(source_name, ".node_palette_", 14) == 0) {
    /* Find the original palette name after the last underscore */
    const char *last_underscore = strrchr(source_name, '_');
    if (last_underscore) {
      source_name = last_underscore + 1;
    }
  }
  
  snprintf(palette_name, sizeof(palette_name), ".node_palette_%s_%p", source_name, (void*)node_name);
  
  Palette *node_palette = (Palette *)BKE_id_new(bmain, ID_PAL, palette_name);
  
  /* Copy all colors from source palette */
  BLI_listbase_clear(&node_palette->colors);
  LISTBASE_FOREACH(PaletteColor *, src_color, &source->colors) {
    PaletteColor *new_color = (PaletteColor *)MEM_callocN(sizeof(PaletteColor), "palette color");
    copy_v3_v3(new_color->rgb, src_color->rgb);
    new_color->value = src_color->value;
    BLI_addtail(&node_palette->colors, new_color);
  }
  
  /* Mark as a fake user so it doesn't get deleted */
  id_fake_user_set(&node_palette->id);
  
  return node_palette;
}

/* Sync node's palette colors from the source palette when source changes */
static void sync_node_palette_from_source(bContext *C, bNode *node, NodeShaderColorPalette *data)
{
  if (!data->source_palette || !data->palette) return;
  
  Palette *source = data->source_palette;
  Palette *node_palette = data->palette;
  
  /* Simple version tracking - compare color count as a proxy for changes */
  int source_color_count = BLI_listbase_count(&source->colors);
  int current_version = source_color_count; /* Simplified - in production could use proper versioning */
  
  if (current_version != data->last_sync_version) {
    /* Source palette has changed - update our copy */
    
    /* Clear existing colors */
    BLI_freelistN(&node_palette->colors);
    
    /* Copy all colors from source */
    LISTBASE_FOREACH(PaletteColor *, src_color, &source->colors) {
      PaletteColor *new_color = (PaletteColor *)MEM_callocN(sizeof(PaletteColor), "palette color");
      copy_v3_v3(new_color->rgb, src_color->rgb);
      new_color->value = src_color->value;
      BLI_addtail(&node_palette->colors, new_color);
    }
    
    /* Update sync version */
    data->last_sync_version = current_version;
    
    /* Clamp palette active_color to valid range */
    int max_color = max_ii(0, BLI_listbase_count(&node_palette->colors) - 1);
    node_palette->active_color = min_ii(node_palette->active_color, max_color);
    
    /* Trigger updates */
    DEG_id_tag_update(&node_palette->id, ID_RECALC_PARAMETERS);
  }
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
  data->source_palette = nullptr;
  data->last_sync_version = 0;
  
  node->storage = data;
}

static void node_shader_draw_color_palette(uiLayout *layout, bContext *C, PointerRNA *ptr)
{
  bNode *node = (bNode *)ptr->data;
  NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
  
  /* Show palette selector */
  uiItemR(layout, ptr, "palette", UI_ITEM_NONE, nullptr, ICON_COLOR);
  
  /* If we have a palette, sync from source and show color grid */
  if (data->palette != nullptr) {
    /* Sync from source palette if needed */
    sync_node_palette_from_source(C, node, data);
    
    Palette *palette = data->palette;
    if (BLI_listbase_count(&palette->colors) > 0) {
      
      /* Show active color info (using palette's active_color) */
      uiLayout *row = uiLayoutRow(layout, true);
      uiItemL(row, "Active Color:", ICON_NONE);
      char active_label[32];
      snprintf(active_label, sizeof(active_label), "%d", palette->active_color + 1);
      uiItemL(row, active_label, ICON_RADIOBUT_ON);
      
      /* Use the exact same approach as interface_templates.cc uiTemplatePalette */
      uiLayout *col = uiLayoutColumn(layout, true);
      uiLayoutRow(col, true);
      
      int row_cols = 0, col_id = 0;
      const int cols_per_row = 3; /* 3x3 grid */
      LISTBASE_FOREACH(PaletteColor *, color, &palette->colors) {
        if (col_id >= 9) break; /* Limit to 3x3 grid */
        
        if (row_cols >= cols_per_row) {
          uiLayoutRow(col, true);
          row_cols = 0;
        }

        uiBlock *block = uiLayoutGetBlock(col);
        PointerRNA color_ptr = RNA_pointer_create(&palette->id, &RNA_PaletteColor, color);
        uiButColor *color_but = (uiButColor *)uiDefButR(block,
                                                        UI_BTYPE_COLOR,
                                                        0,
                                                        "",
                                                        0,
                                                        0,
                                                        UI_UNIT_X * 2.0f,
                                                        UI_UNIT_Y * 2.0f,
                                                        &color_ptr,
                                                        "color",
                                                        -1,
                                                        0.0,
                                                        1.0,
                                                        "");
        color_but->is_pallete_color = true;
        color_but->palette_color_index = col_id;
        
        /* Set callback to trigger material updates when color changes */
        UI_but_func_set((uiBut *)color_but, palette_color_changed_callback, node, nullptr);
        row_cols++;
        col_id++;
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
    /* Use palette's active_color instead of our custom tracking */
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
  NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
  if (!data) return;
  
  /* Check if user assigned a new palette */
  if (data->palette && !data->source_palette) {
    /* User just assigned a palette - this becomes our source palette */
    /* Make sure we're getting the original palette, not a node copy */
    Palette *selected_palette = data->palette;
    
    /* Check if this is already a node copy (starts with ".node_palette_") */
    if (strncmp(selected_palette->id.name + 2, ".node_palette_", 14) == 0) {
      /* This is a node copy - we shouldn't use it as source, skip this update */
      return;
    }
    
    data->source_palette = selected_palette;
    
    /* Create a node-specific copy */
    Main *bmain = G.main;
    if (bmain) {
      Palette *node_copy = create_node_palette_copy(bmain, data->source_palette, node->name);
      if (node_copy) {
        data->palette = node_copy;
        data->palette->active_color = 0; /* Default to first color */
        data->last_sync_version = BLI_listbase_count(&data->source_palette->colors);
      }
    }
  }
  else if (data->palette && data->source_palette) {
    /* We already have a setup - check if user changed the source palette reference */
    Palette *selected_palette = data->palette;
    
    /* Check if user selected a different original palette (not a node copy) */
    if (strncmp(selected_palette->id.name + 2, ".node_palette_", 14) != 0 && 
        selected_palette != data->source_palette) {
      /* User selected a different source palette */
      data->source_palette = selected_palette;
      
      /* Create a new node-specific copy */
      Main *bmain = G.main;
      if (bmain) {
        Palette *node_copy = create_node_palette_copy(bmain, data->source_palette, node->name);
        if (node_copy) {
          data->palette = node_copy;
          data->palette->active_color = 0; /* Default to first color */
          data->last_sync_version = BLI_listbase_count(&data->source_palette->colors);
        }
      }
    }
  }
  else if (!data->palette && data->source_palette) {
    /* User cleared the palette */
    data->source_palette = nullptr;
    data->last_sync_version = 0;
  }
  
  /* Standard node update */
}

/* Custom node cleanup function */
static void node_shader_free_color_palette(bNode *node)
{
  NodeShaderColorPalette *data = (NodeShaderColorPalette *)node->storage;
  if (data && data->palette) {
    /* If this is a node-specific copy, remove fake user so it can be deleted */
    if (strncmp(data->palette->id.name + 2, ".node_palette_", 14) == 0) {
      id_fake_user_clear(&data->palette->id);
    }
  }
  
  /* Standard cleanup */
  if (node->storage) {
    MEM_freeN(node->storage);
    node->storage = nullptr;
  }
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
      &ntype, "NodeShaderColorPalette", file_ns::node_shader_free_color_palette, node_copy_standard_storage);
  ntype.gpu_fn = file_ns::node_shader_gpu_color_palette;
  ntype.initfunc = file_ns::node_shader_init_color_palette;
  ntype.updatefunc = file_ns::node_shader_update_color_palette;
  ntype.draw_buttons = file_ns::node_shader_draw_color_palette;

  nodeRegisterType(&ntype);
}
