// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_POINT_F_H_
#define UI_GFX_POINT_F_H_

#include <string>

#include "ui/base/ui_export.h"
#include "ui/gfx/point_base.h"
#include "ui/gfx/vector2d_f.h"

namespace gfx {

// A floating version of gfx::Point.
class UI_EXPORT PointF : public PointBase<PointF, float, Vector2dF> {
 public:
  PointF();
  PointF(float x, float y);
  ~PointF();

  PointF Scale(float scale) const WARN_UNUSED_RESULT {
    return Scale(scale, scale);
  }

  PointF Scale(float x_scale, float y_scale) const WARN_UNUSED_RESULT {
    return PointF(x() * x_scale, y() * y_scale);
  }

  // Returns a string representation of point.
  std::string ToString() const;
};

inline bool operator==(const PointF& lhs, const PointF& rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator!=(const PointF& lhs, const PointF& rhs) {
  return !(lhs == rhs);
}

inline PointF operator+(const PointF& lhs, const Vector2dF& rhs) {
  return lhs.Add(rhs);
}

inline PointF operator-(const PointF& lhs, const Vector2dF& rhs) {
  return lhs.Subtract(rhs);
}

inline Vector2dF operator-(const PointF& lhs, const PointF& rhs) {
  return lhs.OffsetFrom(rhs);
}

inline PointF PointAtOffsetFromOrigin(const Vector2dF& offset_from_origin) {
  return PointF(offset_from_origin.x(), offset_from_origin.y());
}

#if !defined(COMPILER_MSVC)
extern template class PointBase<PointF, float, Vector2dF>;
#endif

}  // namespace gfx

#endif  // UI_GFX_POINT_F_H_
