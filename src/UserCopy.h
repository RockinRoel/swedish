// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <Wt/WColor.h>
#include <Wt/WString.h>

namespace swedish {

struct UserCopy {
  long long id;
  Wt::WString name;
  Wt::WColor color;
};

}
