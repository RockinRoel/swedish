#include "PuzzleUploader.h"

#include "../model/Square.h"

#include <Wt/WApplication.h>
#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WImage.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WRasterImage.h>
#include <Wt/WRectF.h>
#include <Wt/WTemplate.h>
#include <Wt/WTransform.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

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

inline double luminance(const unsigned char * const buf,
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

Wt::WRectF determineSquare(const std::vector<unsigned char> &buf,
                           const int w,
                           const int h,
                           const int x,
                           const int y)
{
  std::vector<bool> visited(buf.size(), false);
  const double px_l = luminance(buf.data(), w, x, y);

  struct QueueEl {
    int x, y; // x, y position
    double l; // luminance
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

    const double l = luminance(buf.data(), w, cur_x, cur_y);
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

std::vector<swedish::Square> determineSquares(const std::vector<unsigned char> &buf,
                                              const int w,
                                              const int h,
                                              const int x,
                                              const int y)
{
  std::vector<swedish::Square> result;

  result.push_back({determineSquare(buf, w, h, x, y), 0 , 0});
  const Wt::WRectF &rect = result[0].rect;

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

  while (!queue.empty()) {
    auto p = queue.front();
    queue.pop_front();

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

    auto it = std::find_if(begin(result), end(result), [point=Wt::WPointF(cur_x, cur_y)](const swedish::Square &other) {
      return other.rect.contains(point);
    });
    if (it != end(result))
      continue;

    const Wt::WRectF sq = determineSquare(buf, w, h, cur_x, cur_y);
    const Wt::WPointF c = sq.center();

    const double area = sq.width() * sq.height();

    if (area < 0.7 * prev_area ||
        area > 1.3 * prev_area)
      continue;

    result.push_back({sq, row, col});

    const int new_c_x = static_cast<int>(c.x());
    const int new_c_y = static_cast<int>(c.y());
    const int new_r_w = static_cast<int>(sq.width());
    const int new_r_h = static_cast<int>(sq.height());
    queue.push_back({new_c_x - new_r_w, new_c_y, area, row, col - 1});
    queue.push_back({new_c_x, new_c_y - new_r_h, area, row - 1, col});
    queue.push_back({new_c_x + new_r_w, new_c_y, area, row, col + 1});
    queue.push_back({new_c_x, new_c_y + new_r_h, area, row + 1, col});

    std::cout << "QUEUE SIZE: " << queue.size() << '\n' << std::flush;
  }

  return result;
}

}

namespace swedish {

PuzzleUploader::PuzzleUploader()
  : Wt::WCompositeWidget(std::make_unique<Wt::WTemplate>(tr("tpl.swedish.puzzle_uploader"))),
    image_(impl()->bindNew<Wt::WImage>("puzzle_image"))
{
  auto img = createImage(Rotation::Clockwise90);
  image_->setImageLink(Wt::WLink(img));

  image_->clicked().connect(this, &PuzzleUploader::handleClicked);
}

PuzzleUploader::~PuzzleUploader()
{ }

Wt::WTemplate *PuzzleUploader::impl()
{
  return static_cast<Wt::WTemplate*>(implementation());
}

void PuzzleUploader::handleClicked(const Wt::WMouseEvent &evt)
{
  auto img = fillImage(createImage(Rotation::Clockwise90), Wt::WPointF(evt.widget().x, evt.widget().y));
  image_->setImageLink(Wt::WLink(img));
}

std::shared_ptr<Wt::WRasterImage> PuzzleUploader::createImage(Rotation rotation) const
{
  Wt::WApplication *app = Wt::WApplication::instance();
  Wt::WPainter::Image img(app->docRoot() + "/puzzle3.jpg", app->docRoot() + "/puzzle3.jpg");
  int w = img.width();
  int h = img.height();
  if (rotation == Rotation::Clockwise90 ||
      rotation == Rotation::Anticlockwise90) {
    std::swap(w, h);
  }

  auto result = std::make_shared<Wt::WRasterImage>("jpg", w, h);

  {
    Wt::WPainter painter(result.get());

    if (rotation == Rotation::Clockwise90) {
      painter.translate(w, 0);
      painter.rotate(90);
    } else if (rotation == Rotation::Clockwise180) {
      painter.translate(w, h);
      painter.rotate(180);
    } else if (rotation == Rotation::Anticlockwise90) {
      painter.translate(0, h);
      painter.rotate(-90);
    }

    painter.drawImage(Wt::WPointF(0, 0), img);
  }

  return result;
}

std::shared_ptr<Wt::WRasterImage> PuzzleUploader::fillImage(std::shared_ptr<Wt::WRasterImage> image,
                                                            const Wt::WPointF &point)
{
  std::vector<bool> visited;
  std::vector<unsigned char> rgbaPixels;

  const int w = static_cast<int>(image->width().toPixels());
  const int h = static_cast<int>(image->height().toPixels());
  const int x = static_cast<int>(point.x());
  const int y = static_cast<int>(point.y());

  if (x < 0 || x >= w)
    return image;
  if (y < 0 || y >= h)
    return image;

  rgbaPixels.resize(static_cast<std::size_t>(w * h * 4));
  visited.resize(static_cast<std::size_t>(w * h), false);
  image->getPixels(rgbaPixels.data());

  {
    Wt::WPainter painter(image.get());

    painter.setPen(Wt::PenStyle::None);
    painter.setBrush(Wt::StandardColor::Black);

    const auto squares = determineSquares(rgbaPixels, w, h, x, y);
    for (const auto &square : squares) {
      const Wt::WRectF sq = square.rect;
      const int row = square.row;
      const int col = square.col;
      painter.drawText(sq,
                       Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Middle,
                       Wt::TextFlag::SingleLine,
                       Wt::utf8("({1},{2})").arg(row).arg(col));
    }
  }

  return image;
}

PuzzleUploader::Rotation PuzzleUploader::nextClockwise(Rotation rotation)
{
  return static_cast<Rotation>((static_cast<int>(rotation) + 1) % 4);
}

PuzzleUploader::Rotation PuzzleUploader::nextAntiClockwise(Rotation rotation)
{
  return static_cast<Rotation>((static_cast<int>(rotation) + 4 - 1) % 4);
}

}
