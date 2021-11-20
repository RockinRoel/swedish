// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SWEDISH_LAYOUT_H_
#define SWEDISH_LAYOUT_H_

#include <Wt/WCompositeWidget.h>

namespace swedish {

class Layout final : public Wt::WCompositeWidget {
public:
  Layout();
  virtual ~Layout() override;

private:
  Wt::WTemplate *impl();
};

}

#endif // SWEDISH_LAYOUT_H_
