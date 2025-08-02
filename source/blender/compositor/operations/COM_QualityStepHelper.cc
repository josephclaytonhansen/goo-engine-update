/* SPDX-FileCopyrightText: 2011 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "COM_QualityStepHelper.h"

namespace blender::compositor {

QualityStepHelper::QualityStepHelper()
{
  step_ = 1;
  offsetadd_ = 4;
}

void QualityStepHelper::init_execution(QualityHelper helper)
{
  switch (helper) {
    case COM_QH_INCREASE:
      step_ = 1;
      offsetadd_ = 1;
      break;
    case COM_QH_MULTIPLY:
      step_ = 1;
      offsetadd_ = 4;
      break;
  }
}

}  // namespace blender::compositor
