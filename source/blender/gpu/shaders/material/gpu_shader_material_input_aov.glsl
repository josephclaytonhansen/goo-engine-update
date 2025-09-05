/* SPDX-FileCopyrightText: 2020-2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

void node_input_aov(float hash, out vec4 color, out float alpha)
{
  /* For now, return default values. This would need to be connected to
   * an AOV texture sampling system in a full implementation. */
  
  /* Hash-based color generation for testing */
  uint hash_uint = floatBitsToUint(hash);
  float r = float((hash_uint) & 0xFFu) / 255.0;
  float g = float((hash_uint >> 8u) & 0xFFu) / 255.0;
  float b = float((hash_uint >> 16u) & 0xFFu) / 255.0;
  
  color = vec4(r, g, b, 1.0);
  alpha = 1.0;
}
