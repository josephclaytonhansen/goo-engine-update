/* SPDX-FileCopyrightText: 2024 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

namespace blender::nodes::socket_usage_inference {

struct SocketUsage {
  bool is_used = false;
  bool is_visible = true;
};

}  // namespace blender::nodes::socket_usage_inference
