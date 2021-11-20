// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "model/Puzzle.h"
#include "model/Session.h"

#include <Wt/WIOService.h>

#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace swedish {

class SharedSession final : public std::enable_shared_from_this<SharedSession> {
public:
  SharedSession(Wt::WIOService *ioService,
                std::unique_ptr<Wt::Dbo::SqlConnection> conn);

  ~SharedSession();

  void startTimer();
  void stopTimer();

  // returns (character, userid)
  std::pair<Character, long long> charAt(long long puzzle,
                                         std::pair<int, int> cellRef) const;

  // returns the old value (character, userid),
  // optional since maybe this does nothing
  std::optional<std::pair<Character, long long>> updateChar(long long puzzle,
                                                            std::pair<int, int> cellRef,
                                                            Character character,
                                                            long long user);

private:
  Wt::WIOService *ioService_;
  std::unique_ptr<boost::asio::steady_timer> timer_;
  mutable Session session_;
  mutable std::mutex mutex_;
  mutable std::vector<Wt::Dbo::ptr<Puzzle>> puzzles_;
  std::atomic_bool terminated_;

  // NOTE: NEED LOCK BEFORE CALLING THIS
  Wt::Dbo::ptr<Puzzle> getPuzzle(long long puzzle) const;
  void timeout(boost::system::error_code errc);
  void sync(bool last);
};

}
