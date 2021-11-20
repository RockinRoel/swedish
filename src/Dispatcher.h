// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <Wt/WSignal.h>

#include "model/Puzzle.h"

#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <Wt/WColor.h>

namespace swedish {

class Subscriber;

struct UserCursor {
  long long puzzleId = -1;
  long long userId = -1;
  std::pair<int, int> cellRef = {-1, -1};
  Wt::Orientation direction = Wt::Orientation::Horizontal;
};

// one dispatcher in the entire program, to send events
class Dispatcher final {
public:
  Dispatcher(Wt::WServer *server);

  void addSubsriber(Subscriber &subscriber);
  void removeSubscriber(Subscriber &subscriber);

  void notifyUserAdded(Subscriber &self,
                       long long id,
                       const Wt::WString &name,
                       const Wt::WColor &color);

  void notifyUserChangedColor(Subscriber &self,
                              long long id,
                              const Wt::WColor &color);

  void notifyCellValueChanged(Subscriber &self,
                              long long puzzleId,
                              std::pair<int, int> cellRef);

  void notifyCursorMoved(Subscriber &self,
                         long long puzzleId,
                         long long user,
                         std::pair<int, int> cellRef,
                         Wt::Orientation direction);

  std::vector<UserCursor> userPositions() const;

private:
  std::mutex subscriberMutex_;
  mutable std::mutex positionMutex_;
  Wt::WServer *server_;
  std::vector<std::reference_wrapper<Subscriber>> subscribers_;
  std::vector<UserCursor> userPositions_;
};

// one subsciber per WApplication, to receive events
class Subscriber final {
public:
  Subscriber(const std::string &sessionId);

  const std::string & sessionId() const { return sessionId_; }

  Wt::Signal<long long, const Wt::WString &, const Wt::WColor &> &userAdded() { return userAdded_; }
  Wt::Signal<long long, const Wt::WColor &> &userChangedColor() { return userChangedColor_; }
  Wt::Signal<long long, std::pair<int, int>> &cellValueChanged() { return cellValueChanged_; }
  Wt::Signal<long long, long long, std::pair<int, int>, Wt::Orientation> &cursorMoved() { return cursorMoved_; }

private:
  std::string sessionId_;
  Wt::Signal<long long, const Wt::WString &, const Wt::WColor &> userAdded_;
  Wt::Signal<long long, const Wt::WColor &> userChangedColor_;
  Wt::Signal<long long, std::pair<int, int>> cellValueChanged_;
  Wt::Signal<long long, long long, std::pair<int, int>, Wt::Orientation> cursorMoved_;
};

}
