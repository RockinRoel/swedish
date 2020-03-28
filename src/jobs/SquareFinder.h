#ifndef SWEDISH_SQUAREFINDER_H_
#define SWEDISH_SQUAREFINDER_H_

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

  SquareFinder(Puzzle &puzzle,
               Rotation rotation,
               int x, int y);
  ~SquareFinder();

  Wt::Signal<Status> &statusChanged() { return statusChanged_; }

private:
  struct Square {
    Wt::WRectF rect;
    int row = 0, col = 0;
  };

  Puzzle &puzzle_;
  std::thread thread_;
  std::promise<void> stopSignal_;
  std::future<void> stopFuture_;
  std::vector<Square> squares_;
  Wt::Signal<Status> statusChanged_;
  Wt::WApplication *app_;
  Wt::WServer *server_;

  void abort();
  bool stopRequested();
  Wt::WRectF determineSquare(const std::vector<unsigned char> &buf,
                             const int w,
                             const int h,
                             const int x,
                             const int y);
  void determineSquares(const std::vector<unsigned char> &buf,
                        const int w,
                        const int h,
                        const int x,
                        const int y);
  void populatePuzzle();
  static std::vector<unsigned char> extractImageData(const std::string &path,
                                                     Rotation rotation,
                                                     int &w, int &h);
  void updateStatus(Status status);
};

}

#endif // SWEDISH_SQUAREFINDER_H_
