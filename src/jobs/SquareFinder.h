// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "../Rotation.h"
#include "../model/Puzzle.h"

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace swedish {

class SquareFinder final : public Wt::WObject {
public:
  struct ReadingImage {};
  struct Processing {
    int queueSize = 0;
  };
  struct PopulatingPuzzle {};
  struct Done {};
  using Status = std::variant<ReadingImage, Processing, PopulatingPuzzle, Done>;

  SquareFinder(Puzzle &puzzle, int x, int y);
  ~SquareFinder() override;

  void start();

  Wt::Signal<Status> &statusChanged() { return statusChanged_; }

private:
  struct Square {
    Wt::WRectF rect;
    int row = 0, col = 0;
  };

  Puzzle &puzzle_;
  int x_, y_;
  std::thread thread_;
  std::promise<void> stopSignal_;
  std::future<void> stopFuture_;
  std::vector<Square> squares_;
  Wt::Signal<Status> statusChanged_;
  Wt::WApplication *app_;
  Wt::WServer *server_;

  void abort();
  bool stopRequested();
  Wt::WRectF determineSquare(const std::vector<unsigned char> &buf, int w, int h, int x, int y);
  void determineSquares(const std::vector<unsigned char> &buf, int w, int h, int x, int y);
  void populatePuzzle();
  static std::vector<unsigned char> extractImageData(const std::string &path,
                                                     Rotation rotation,
                                                     int &w, int &h);
  void updateStatus(Status status);
};

}
