#include "SquareFinder.h"

#include <Wt/WApplication.h>
#include <Wt/WColor.h>
#include <Wt/WPainter.h>
#include <Wt/WPointF.h>
#include <Wt/WRasterImage.h>
#include <Wt/WRectF.h>
#include <Wt/WServer.h>

#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iostream>
#include <optional>

namespace {

inline Wt::WColor bufpix(const unsigned char * const buf,
                         const int width,
                         const int x,
                         const int y)
{
  const std::size_t offset = static_cast<std::size_t>((y * width + x) * 4);
  return Wt::WColor(static_cast<int>(*(buf + offset)),
                    static_cast<int>(*(buf + offset + 1)),
                    static_cast<int>(*(buf + offset + 2)));
}

inline double lightness(const unsigned char * const buf,
                        const int width,
                        const int x,
                        const int y)
{
  double total = 0;
  double hsl[3];
  for (int x0 = x - 1; x0 <= x + 1; ++x0) {
    for (int y0 = y - 1; y0 <= y + 1; ++y0) {
      bufpix(buf, width, x0, y0).toHSL(hsl);
      total += hsl[2];
    }
  }
  return total / 9.0;
}

}

namespace swedish {

SquareFinder::SquareFinder(Puzzle &puzzle,
                           const int x,
                           const int y)
  : puzzle_(puzzle),
    x_(x),
    y_(y),
    stopFuture_(stopSignal_.get_future()),
    app_(nullptr),
    server_(nullptr)
{
  app_ = Wt::WApplication::instance();
  server_ = Wt::WServer::instance();
}

SquareFinder::~SquareFinder()
{
  abort();

  if (thread_.joinable()) {
    thread_.join();
  }
}

void SquareFinder::start()
{
  Wt::WApplication *app = Wt::WApplication::instance();
  boost::filesystem::path docRoot = app->docRoot();
  std::string path = (docRoot / puzzle_.path).string();
  thread_ = std::thread([this, path]{
    int w, h;
    updateStatus(ReadingImage());
    std::vector<unsigned char> buf = extractImageData(path, puzzle_.rotation, w, h);
    updateStatus(Processing { 0 });
    determineSquares(buf, w, h, x_, y_);
    updateStatus(PopulatingPuzzle());
    if (stopRequested()) {
      return;
    }
    populatePuzzle();
    updateStatus(Done());
  });
}

void SquareFinder::abort()
{
  stopSignal_.set_value();
}

bool SquareFinder::stopRequested()
{
  if (stopFuture_.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
    return false;
  return true;
}

Wt::WRectF SquareFinder::determineSquare(const std::vector<unsigned char> &buf,
                                         const int w,
                                         const int h,
                                         const int x,
                                         const int y)
{
  std::vector<bool> visited(buf.size(), false);
  const double px_l = lightness(buf.data(), w, x, y);

  struct QueueEl {
    int x, y; // x, y position
    double l; // lightness
  };

  std::deque<QueueEl> queue;
  queue.push_back({x - 1, y, px_l});
  queue.push_back({x, y - 1, px_l});
  queue.push_back({x + 1, y, px_l});
  queue.push_back({x, y + 1, px_l});

  visited[static_cast<std::size_t>(y * w + x)] = true;

  int min_x = x;
  int max_x = x;
  int min_y = y;
  int max_y = y;
  while (!queue.empty()) {
    auto p = queue.front();
    queue.pop_front();

    const int cur_x = p.x;
    const int cur_y = p.y;
    const double prev_l = p.l;

    if (cur_x < 1 ||
        cur_x >= w - 1 ||
        cur_y < 1 ||
        cur_y >= h - 1)
      continue; // out of bounds

    if (visited[static_cast<std::size_t>(cur_y * w + cur_x)])
      continue; // already visited

    visited[static_cast<std::size_t>(cur_y * w + cur_x)] = true;

    const double l = lightness(buf.data(), w, cur_x, cur_y);
    if (std::abs(l - prev_l) < 0.01) {
      queue.push_back({cur_x - 1, cur_y, l});
      queue.push_back({cur_x, cur_y - 1, l});
      queue.push_back({cur_x + 1, cur_y, l});
      queue.push_back({cur_x, cur_y + 1, l});

      if (cur_x < min_x)
        min_x = cur_x;
      if (cur_x > max_x)
        max_x = cur_x;
      if (cur_y < min_y)
        min_y = cur_y;
      if (cur_y > max_y)
        max_y = cur_y;
    }
  }

  return Wt::WRectF(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}

void SquareFinder::determineSquares(const std::vector<unsigned char> &buf,
                                    const int w,
                                    const int h,
                                    const int x,
                                    const int y)
{
  squares_.push_back({determineSquare(buf, w, h, x, y), 0 , 0});
  const Wt::WRectF &rect = squares_[0].rect;

  const Wt::WPointF center = rect.center();
  const int c_x = static_cast<int>(center.x());
  const int c_y = static_cast<int>(center.y());
  const int r_w = static_cast<int>(rect.width());
  const int r_h = static_cast<int>(rect.height());
  const double r_area = rect.width() * rect.height();

  struct QueueEl {
    int x, y;
    double area;
    int row, col;
  };

  std::deque<QueueEl> queue;
  queue.push_back({c_x - r_w, c_y, r_area, 0, -1});
  queue.push_back({c_x, c_y - r_h, r_area, -1, 0});
  queue.push_back({c_x + r_w, c_y, r_area, 0, 1});
  queue.push_back({c_x, c_y + r_h, r_area, 1, 0});

  updateStatus(Processing { static_cast<int>(queue.size()) });

  while (!queue.empty()) {
    if (stopRequested())
      return;

    auto p = queue.front();
    queue.pop_front();

    updateStatus(Processing { static_cast<int>(queue.size()) });

    const int cur_x = p.x;
    const int cur_y = p.y;
    const double prev_area = p.area;
    const int row = p.row;
    const int col = p.col;

    if (cur_x < 1 ||
        cur_x >= w - 1 ||
        cur_y < 1 ||
        cur_y >= h - 1)
      continue;

    auto it = std::find_if(begin(squares_), end(squares_), [point=Wt::WPointF(cur_x, cur_y)](const Square &other) {
      return other.rect.contains(point);
    });
    if (it != end(squares_))
      continue;

    const Wt::WRectF sq = determineSquare(buf, w, h, cur_x, cur_y);

    if (sq.isNull())
      return; // stop requested

    const Wt::WPointF c = sq.center();

    const double area = sq.width() * sq.height();

    if (area < 0.7 * prev_area ||
        area > 1.3 * prev_area)
      continue;

    squares_.push_back({sq, row, col});

    const int new_c_x = static_cast<int>(c.x());
    const int new_c_y = static_cast<int>(c.y());
    const int new_r_w = static_cast<int>(sq.width());
    const int new_r_h = static_cast<int>(sq.height());
    queue.push_back({new_c_x - new_r_w, new_c_y, area, row, col - 1});
    queue.push_back({new_c_x, new_c_y - new_r_h, area, row - 1, col});
    queue.push_back({new_c_x + new_r_w, new_c_y, area, row, col + 1});
    queue.push_back({new_c_x, new_c_y + new_r_h, area, row + 1, col});

    updateStatus(Processing { static_cast<int>(queue.size()) });
  }
}

void SquareFinder::populatePuzzle()
{
  if (squares_.empty())
    return;

  int minRow = squares_[0].row;
  int maxRow = squares_[0].row;
  int minCol = squares_[0].col;
  int maxCol = squares_[0].col;
  for (std::size_t i = 1; i < squares_.size(); ++i) {
    const Square &square = squares_[i];
    minRow = std::min(minRow, square.row);
    maxRow = std::max(maxRow, square.row);
    minCol = std::min(minCol, square.col);
    maxCol = std::max(maxCol, square.col);
  }

  const int rowOffset = -minRow;
  const int colOffset = -minCol;
  const int nRows = maxRow - minRow + 1;
  const int nCols = maxCol - minCol + 1;

  puzzle_.rows_.clear();

  for (int r = 0; r < nRows; ++r) {
    auto &row = puzzle_.rows_.emplace_back();
    for (int c = 0; c < nCols; ++c) {
      row.emplace_back();
    }
  }

  for (auto &square : squares_) {
    Puzzle::Row &row = puzzle_.rows_[static_cast<std::size_t>(square.row + rowOffset)];
    Cell &cell = row[static_cast<std::size_t>(square.col + colOffset)];
    cell.square = square.rect;
  }
}

std::vector<unsigned char> SquareFinder::extractImageData(const std::string &path,
                                                          Rotation rotation,
                                                          int &w, int &h)
{
  Wt::WPainter::Image img(path, path);
  w = img.width();
  h = img.height();
  if (rotation == Rotation::Clockwise90 ||
      rotation == Rotation::AntiClockwise90) {
    std::swap(w, h);
  }

  Wt::WRasterImage rasterImage("png", w, h);

  {
    Wt::WPainter painter(&rasterImage);

    if (rotation == Rotation::Clockwise90) {
      painter.translate(w, 0);
      painter.rotate(90);
    } else if (rotation == Rotation::Clockwise180) {
      painter.translate(w, h);
      painter.rotate(180);
    } else if (rotation == Rotation::AntiClockwise90) {
      painter.translate(0, h);
      painter.rotate(-90);
    }

    painter.drawImage(Wt::WPointF(0, 0), img);
  }

  std::vector<unsigned char> rgbaPixels;
  rgbaPixels.resize(static_cast<std::size_t>(w * h * 4));
  rasterImage.getPixels(rgbaPixels.data());

  return rgbaPixels;
}

void SquareFinder::updateStatus(Status status)
{
  if (!app_)
    return;

  server_->post(app_->sessionId(), [this, status]{
    statusChanged_(status);
    app_->triggerUpdate();
  });
}

}
