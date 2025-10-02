/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup RNA
 */

#include <cstdlib>

#include "BLI_utildefines.h"

#include "RNA_access.hh"
#include "RNA_define.hh"

#include "rna_internal.hh"

#include "WM_types.hh"

#ifdef RNA_RUNTIME

#  include "DNA_brush_types.h"

#  include "BKE_paint.hh"
#  include "BKE_report.hh"
static PaletteColor *rna_Palette_color_new(Palette *palette)
{
  if (ID_IS_LINKED(palette) || ID_IS_OVERRIDE_LIBRARY(palette)) {
    return nullptr;
  }

  PaletteColor *color = BKE_palette_color_add(palette);
  return color;
}

static void rna_Palette_color_remove(Palette *palette, ReportList *reports, PointerRNA *color_ptr)
{
  if (ID_IS_LINKED(palette) || ID_IS_OVERRIDE_LIBRARY(palette)) {
    return;
  }

  PaletteColor *color = static_cast<PaletteColor *>(color_ptr->data);

  if (BLI_findindex(&palette->colors, color) == -1) {
    BKE_reportf(
        reports, RPT_ERROR, "Palette '%s' does not contain color given", palette->id.name + 2);
    return;
  }

  BKE_palette_color_remove(palette, color);

  RNA_POINTER_INVALIDATE(color_ptr);
}

static void rna_Palette_color_clear(Palette *palette)
{
  if (ID_IS_LINKED(palette) || ID_IS_OVERRIDE_LIBRARY(palette)) {
    return;
  }

  BKE_palette_clear(palette);
}

static void rna_PaletteColor_color_update(Main * /*bmain*/, Scene * /*scene*/, PointerRNA *ptr)
{
  PaletteColor *color = static_cast<PaletteColor *>(ptr->data);
  /* Find the palette that contains this color and increment its version */
  ID *id = ptr->owner_id;
  if (id && GS(id->name) == ID_PAL) {
    Palette *palette = reinterpret_cast<Palette *>(id);
    palette->version++;
  }
}

static PointerRNA rna_Palette_active_color_get(PointerRNA *ptr)
{
  Palette *palette = static_cast<Palette *>(ptr->data);
  PaletteColor *color;

  color = static_cast<PaletteColor *>(BLI_findlink(&palette->colors, palette->active_color));

  if (color) {
    return rna_pointer_inherit_refine(ptr, &RNA_PaletteColor, color);
  }

  return rna_pointer_inherit_refine(ptr, nullptr, nullptr);
}

static void rna_Palette_active_color_set(PointerRNA *ptr,
                                         PointerRNA value,
                                         ReportList * /*reports*/)
{
  Palette *palette = static_cast<Palette *>(ptr->data);
  PaletteColor *color = static_cast<PaletteColor *>(value.data);

  /* -1 is ok for an unset index */
  if (color == nullptr) {
    palette->active_color = -1;
  }
  else {
    palette->active_color = BLI_findindex(&palette->colors, color);
  }
}

static bool rna_Palette_color_move_up(Palette *palette, ReportList *reports, PointerRNA *color_ptr)
{
  if (ID_IS_LINKED(palette) || ID_IS_OVERRIDE_LIBRARY(palette)) {
    BKE_report(reports, RPT_ERROR, "Cannot reorder colors in linked or library override palette");
    return false;
  }

  PaletteColor *color = static_cast<PaletteColor *>(color_ptr->data);

  if (BLI_findindex(&palette->colors, color) == -1) {
    BKE_reportf(
        reports, RPT_ERROR, "Palette '%s' does not contain color given", palette->id.name + 2);
    return false;
  }

  return BKE_palette_color_move_up(palette, color);
}

static bool rna_Palette_color_move_down(Palette *palette, ReportList *reports, PointerRNA *color_ptr)
{
  if (ID_IS_LINKED(palette) || ID_IS_OVERRIDE_LIBRARY(palette)) {
    BKE_report(reports, RPT_ERROR, "Cannot reorder colors in linked or library override palette");
    return false;
  }

  PaletteColor *color = static_cast<PaletteColor *>(color_ptr->data);

  if (BLI_findindex(&palette->colors, color) == -1) {
    BKE_reportf(
        reports, RPT_ERROR, "Palette '%s' does not contain color given", palette->id.name + 2);
    return false;
  }

  return BKE_palette_color_move_down(palette, color);
}

static PaletteColor *rna_Palette_color_get_by_index(Palette *palette, int index)
{
  return BKE_palette_color_get_by_index(palette, index);
}

#else

/* palette.colors */
static void rna_def_palettecolors(BlenderRNA *brna, PropertyRNA *cprop)
{
  StructRNA *srna;
  PropertyRNA *prop;

  FunctionRNA *func;
  PropertyRNA *parm;

  RNA_def_property_srna(cprop, "PaletteColors");
  srna = RNA_def_struct(brna, "PaletteColors", nullptr);
  RNA_def_struct_sdna(srna, "Palette");
  RNA_def_struct_ui_text(srna, "Palette Splines", "Collection of palette colors");

  func = RNA_def_function(srna, "new", "rna_Palette_color_new");
  RNA_def_function_ui_description(func, "Add a new color to the palette");
  parm = RNA_def_pointer(func, "color", "PaletteColor", "", "The newly created color");
  RNA_def_function_return(func, parm);

  func = RNA_def_function(srna, "remove", "rna_Palette_color_remove");
  RNA_def_function_ui_description(func, "Remove a color from the palette");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  parm = RNA_def_pointer(func, "color", "PaletteColor", "", "The color to remove");
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED | PARM_RNAPTR);
  RNA_def_parameter_clear_flags(parm, PROP_THICK_WRAP, ParameterFlag(0));

  func = RNA_def_function(srna, "clear", "rna_Palette_color_clear");
  RNA_def_function_ui_description(func, "Remove all colors from the palette");

  func = RNA_def_function(srna, "move_up", "rna_Palette_color_move_up");
  RNA_def_function_ui_description(func, "Move a color up in the palette (toward index 0)");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  parm = RNA_def_pointer(func, "color", "PaletteColor", "", "The color to move");
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED | PARM_RNAPTR);
  RNA_def_parameter_clear_flags(parm, PROP_THICK_WRAP, ParameterFlag(0));
  parm = RNA_def_boolean(func, "success", false, "Success", "True if color was moved");
  RNA_def_function_return(func, parm);

  func = RNA_def_function(srna, "move_down", "rna_Palette_color_move_down");
  RNA_def_function_ui_description(func, "Move a color down in the palette (toward end)");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  parm = RNA_def_pointer(func, "color", "PaletteColor", "", "The color to move");
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED | PARM_RNAPTR);
  RNA_def_parameter_clear_flags(parm, PROP_THICK_WRAP, ParameterFlag(0));
  parm = RNA_def_boolean(func, "success", false, "Success", "True if color was moved");
  RNA_def_function_return(func, parm);

  func = RNA_def_function(srna, "get", "rna_Palette_color_get_by_index");
  RNA_def_function_ui_description(func, "Get a color by its index");
  parm = RNA_def_int(func, "index", 0, 0, INT_MAX, "Index", "Index of the color to retrieve", 0, INT_MAX);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_pointer(func, "color", "PaletteColor", "", "The color at the given index");
  RNA_def_function_return(func, parm);

  prop = RNA_def_property(srna, "active", PROP_POINTER, PROP_NONE);
  RNA_def_property_struct_type(prop, "PaletteColor");
  RNA_def_property_pointer_funcs(
      prop, "rna_Palette_active_color_get", "rna_Palette_active_color_set", nullptr, nullptr);
  RNA_def_property_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Active Palette Color", "");
}

static void rna_def_palettecolor(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;

  srna = RNA_def_struct(brna, "PaletteColor", nullptr);
  RNA_def_struct_ui_text(srna, "Palette Color", "");

  prop = RNA_def_property(srna, "color", PROP_FLOAT, PROP_COLOR_GAMMA);
  RNA_def_property_range(prop, 0.0, 1.0);
  RNA_def_property_float_sdna(prop, nullptr, "rgb");
  RNA_def_property_flag(prop, PROP_LIB_EXCEPTION);
  RNA_def_property_array(prop, 3);
  RNA_def_property_ui_text(prop, "Color", "");
  RNA_def_property_update(prop, NC_SCENE | ND_TOOLSETTINGS, "rna_PaletteColor_color_update");

  prop = RNA_def_property(srna, "index", PROP_INT, PROP_NONE);
  RNA_def_property_int_sdna(prop, nullptr, "index");
  RNA_def_property_clear_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Index", "Position of the color in the palette");
}

static void rna_def_palette(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;

  srna = RNA_def_struct(brna, "Palette", "ID");
  RNA_def_struct_ui_text(srna, "Palette", "");
  RNA_def_struct_ui_icon(srna, ICON_COLOR);

  prop = RNA_def_property(srna, "colors", PROP_COLLECTION, PROP_NONE);
  RNA_def_property_struct_type(prop, "PaletteColor");
  rna_def_palettecolors(brna, prop);

  prop = RNA_def_property(srna, "version", PROP_INT, PROP_NONE);
  RNA_def_property_int_sdna(prop, nullptr, "version");
  RNA_def_property_clear_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Version", "Counter incremented when colors are added/removed/reordered");
}

void RNA_def_palette(BlenderRNA *brna)
{
  /* *** Non-Animated *** */
  RNA_define_animate_sdna(false);
  rna_def_palettecolor(brna);
  rna_def_palette(brna);
  RNA_define_animate_sdna(true);
}

#endif
