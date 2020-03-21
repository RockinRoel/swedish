#include "PuzzleUploader.h"

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

#include <chrono>
#include <deque>
#include <memory>
#include <tuple>
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

  std::deque<std::tuple<int, int, double>> queue;
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

    const int cur_x = std::get<0>(p);
    const int cur_y = std::get<1>(p);
    const double prev_l = std::get<2>(p);

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
  Wt::WPainter::Image img(app->docRoot() + "/puzzle.jpg", app->docRoot() + "/puzzle.jpg");
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
    painter.setBrush(Wt::StandardColor::Red);

    painter.drawRect(determineSquare(rgbaPixels, w, h, x, y));
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
