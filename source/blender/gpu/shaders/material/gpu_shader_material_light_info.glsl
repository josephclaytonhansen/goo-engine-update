/* SPDX-FileCopyrightText: 2025 Goo Engine Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

void node_light_info_simple(vec4 light_color,
                            float light_power,
                            out vec4 out_light_color,
                            out float out_light_power)
{
  /* Output the light properties */
  out_light_color = light_color;
  out_light_power = light_power;
}