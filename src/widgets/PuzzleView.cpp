#include "PuzzleView.h"

#include "../UserCopy.h"

#include <Wt/WApplication.h>
#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLength.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPen.h>
#include <Wt/WPushButton.h>
#include <Wt/WVBoxLayout.h>

#include <Wt/Dbo/Dbo.h>

#include <algorithm>
#include <memory>

namespace {

constexpr const double max_zoom = 2.0;
constexpr const double min_zoom = 0.1;

}

namespace swedish {

class PuzzleView::Layer : public Wt::WPaintedWidget {
public:
  Layer(PuzzleView *puzzleView);
  virtual ~Layer() override;

  double zoom() const { return puzzleView_->zoom_; }
  const Puzzle *puzzle() const { return puzzleView_->puzzle_.get(); }

protected:
  PuzzleView *puzzleView_;
};

class PuzzleView::PuzzlePaintedWidget final : public PuzzleView::Layer {
public:
  PuzzlePaintedWidget(PuzzleView *puzzleView);
  virtual ~PuzzlePaintedWidget() override;

protected:
  virtual void paintEvent(Wt::WPaintDevice *paintDevice) override;
};

class PuzzleView::TextLayer final : public PuzzleView::Layer {
public:
  TextLayer(PuzzleView * puzzleView);
  virtual ~TextLayer() override;

protected:
  virtual void paintEvent(Wt::WPaintDevice *paintDevice) override;
};

PuzzleView::Layer::Layer(PuzzleView *puzzleView)
  : puzzleView_(puzzleView)
{ }

PuzzleView::Layer::~Layer()
{ }

PuzzleView::PuzzlePaintedWidget::PuzzlePaintedWidget(PuzzleView *puzzleView)
  : Layer(puzzleView)
{
  auto puzzle = puzzleView->puzzle_;
}

PuzzleView::PuzzlePaintedWidget::~PuzzlePaintedWidget()
{ }

void PuzzleView::PuzzlePaintedWidget::paintEvent(Wt::WPaintDevice *paintDevice)
{
  Wt::WPainter painter(paintDevice);

  auto puzzle = puzzleView_->puzzle_;
  const std::string &path = puzzle->path;
  const Rotation rotation = puzzle->rotation;
  const int w = puzzle->width;
  const int h = puzzle->height;

  painter.scale(zoom(), zoom());

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

  Wt::WApplication *app = Wt::WApplication::instance();

  Wt::WPainter::Image img(path, app->docRoot() + path);
  painter.drawImage(Wt::WPointF(0.0, 0.0), img);
}

PuzzleView::TextLayer::TextLayer(PuzzleView *puzzleView)
  : Layer(puzzleView)
{
  setPositionScheme(Wt::PositionScheme::Absolute);
  setOffsets(Wt::WLength(0, Wt::LengthUnit::Pixel), Wt::Side::Top | Wt::Side::Left);
}

PuzzleView::TextLayer::~TextLayer()
{ }

void PuzzleView::TextLayer::paintEvent(Wt::WPaintDevice *paintDevice)
{
  Wt::WPainter painter(paintDevice);

  painter.scale(zoom(), zoom());

  painter.setPen(Wt::WPen(Wt::PenStyle::None));
  painter.setBrush(Wt::WBrush(Wt::StandardColor::Red));

  Wt::WFont font;
  font.setFamily(Wt::FontFamily::SansSerif);

  for (const auto &row : puzzle()->rows_) {
    for (const auto &cell : row) {
      if (cell.empty()) {
        continue;
      }

      const Wt::WRectF &square = cell.square;
      std::string str(charToStr(cell.character_));

      double minSize = std::min(square.width(), square.height());
      font.setSize(Wt::WLength(0.7 * minSize));
      painter.setFont(font);

      painter.drawText(square,
                       Wt::AlignmentFlag::Center |
                       Wt::AlignmentFlag::Middle,
                       Wt::TextFlag::SingleLine,
                       Wt::utf8(str));
    }
  }
}

PuzzleView::PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle)
  : Wt::WCompositeWidget(std::make_unique<Wt::WContainerWidget>()),
    puzzle_(puzzle)
{
  Wt::WApplication *app = Wt::WApplication::instance();
  app->useStyleSheet(Wt::WApplication::relativeResourcesUrl() +
                     "font-awesome/css/font-awesome.min.css");

  auto layout = impl()->setLayout(std::make_unique<Wt::WVBoxLayout>());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(3);

  auto top = layout->addWidget(std::make_unique<Wt::WContainerWidget>(), 0);
  auto bottom = layout->addWidget(std::make_unique<Wt::WContainerWidget>(), 1);
  bottom->setPositionScheme(Wt::PositionScheme::Relative);
  bottom->setOverflow(Wt::Overflow::Auto);

  paintedWidget_ = bottom->addNew<PuzzlePaintedWidget>(this);
  textLayer_ = bottom->addNew<TextLayer>(this);

  paintedWidget_->resize(puzzle->width, puzzle->height);
  textLayer_->resize(puzzle->width, puzzle->height);

  zoomInBtn_ = top->addNew<Wt::WPushButton>();
  zoomOutBtn_ = top->addNew<Wt::WPushButton>();

  zoomInBtn_->setOffsets(10, Wt::Side::Top | Wt::Side::Left);
  zoomOutBtn_->setOffsets(10, Wt::Side::Left);
  zoomOutBtn_->setOffsets(46, Wt::Side::Top);

  zoomInBtn_->setTextFormat(Wt::TextFormat::XHTML);
  zoomOutBtn_->setTextFormat(Wt::TextFormat::XHTML);

  zoomInBtn_->setText(Wt::utf8("<i class=\"fa fa-search-plus\"></i> zoom in"));
  zoomOutBtn_->setText(Wt::utf8("<i class=\"fa fa-search-minus\"></i> zoom out"));

  zoomInBtn_->clicked().connect(this, &PuzzleView::zoomIn);
  zoomOutBtn_->clicked().connect(this, &PuzzleView::zoomOut);

  // TODO(Roel): other controls
  // TODO(Roel): clicked
}

PuzzleView::~PuzzleView()
{ }

Wt::WContainerWidget *PuzzleView::impl()
{
  return static_cast<Wt::WContainerWidget *>(implementation());
}

void PuzzleView::zoomIn()
{
  zoom_ = std::min(zoom_ + 0.1, max_zoom);

  paintedWidget_->resize(puzzle_->width * zoom_,
                         puzzle_->height * zoom_);
  textLayer_->resize(puzzle_->width * zoom_,
                     puzzle_->height * zoom_);

  paintedWidget_->update();
}

void PuzzleView::zoomOut()
{
  zoom_ = std::max(zoom_ - 0.1, min_zoom);

  paintedWidget_->resize(puzzle_->width * zoom_,
                         puzzle_->height * zoom_);
  textLayer_->resize(puzzle_->width * zoom_,
                     puzzle_->height * zoom_);

  paintedWidget_->update();
}

}
