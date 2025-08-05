/* SPDX-FileCopyrightText: 2024 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

void view_position_get(out vec3 position)
{
  position = coordinate_camera(g_data.P);
}
