/* SPDX-FileCopyrightText: 2011 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

namespace blender::compositor {

typedef enum QualityHelper {
  COM_QH_INCREASE,
  COM_QH_MULTIPLY,
} QualityHelper;

class QualityStepHelper {
 private:
  int step_;
  int offsetadd_;

 protected:
  void init_execution(QualityHelper helper);

  inline int get_step() const
  {
    return step_;
  }
  inline int get_offset_add() const
  {
    return offsetadd_;
  }

 public:
  QualityStepHelper();
};

}  // namespace blender::compositor
