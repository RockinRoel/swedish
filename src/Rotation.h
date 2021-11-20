// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <cassert>

namespace swedish {

enum class Rotation : int {
  None = 0,
  Clockwise90 = 1,
  Clockwise180 = 2,
  AntiClockwise90 = 3
};

constexpr static inline Rotation nextClockwise(const Rotation rotation) noexcept
{
  return static_cast<Rotation>((static_cast<int>(rotation) + 1) % 4);
}

constexpr static inline Rotation nextAntiClockwise(const Rotation rotation) noexcept
{
  return static_cast<Rotation>((static_cast<int>(rotation) + 4 - 1) % 4);
}

constexpr static inline int rotationToDegrees(const Rotation rotation) noexcept
{
  switch (rotation) {
  case Rotation::None:
    return 0;
  case Rotation::Clockwise90:
    return 90;
  case Rotation::Clockwise180:
    return 180;
  case Rotation::AntiClockwise90:
    return -90;
  }
  assert(false);
  return 0;
}

constexpr static inline Rotation degreesToRotation(const int degrees) noexcept
{
  if (degrees == 0)
    return Rotation::None;
  else if (degrees == 90)
    return Rotation::Clockwise90;
  else if (degrees == 180)
    return Rotation::Clockwise180;
  else if (degrees == -90)
    return Rotation::AntiClockwise90;
  else {
    assert(false);
    return Rotation::None;
  }
}

}
