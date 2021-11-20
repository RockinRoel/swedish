// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <Wt/Dbo/Session.h>

#include <memory>

namespace swedish {

class Session final : public Wt::Dbo::Session {
public:
  explicit Session(std::unique_ptr<Wt::Dbo::SqlConnection> conn);
  explicit Session(Wt::Dbo::SqlConnectionPool &pool);
  ~Session() override;

private:
  void init();
};

}
