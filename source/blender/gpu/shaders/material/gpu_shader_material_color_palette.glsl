/* SPDX-FileCopyrightText: 2025 Goo Engine Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

void node_color_palette_active(vec4 active_color, out vec4 outColor, out float outAlpha)
{
  outColor = active_color;
  outAlpha = active_color.a;
}
