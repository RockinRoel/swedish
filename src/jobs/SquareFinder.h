#ifndef SWEDISH_SQUAREFINDER_H_
#define SWEDISH_SQUAREFINDER_H_

#include "../Rotation.h"
#include "../model/Square.h"

#include <future>
#include <string>
#include <thread>
#include <vector>

namespace swedish {

class SquareFinder final {
public:
  SquareFinder(const std::string &path,
               Rotation rotation,
               int x, int y);
  ~SquareFinder();

  // TODO(Roel): progress updates?
  //  - reading image
  //  - queue size

private:
  std::thread thread_;
  std::promise<void> stopSignal_;
  std::future<void> stopFuture_;
  std::vector<Square> squares_;

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
  static std::vector<unsigned char> extractImageData(const std::string &path,
                                                     Rotation rotation,
                                                     int &w, int &h);
};

}

#endif // SWEDISH_SQUAREFINDER_H_
