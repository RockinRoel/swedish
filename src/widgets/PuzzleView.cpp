#include "PuzzleView.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPushButton.h>

#include <Wt/Dbo/Dbo.h>

#include <algorithm>
#include <memory>

namespace {

constexpr const double max_zoom = 2.0;
constexpr const double min_zoom = 0.1;

}

namespace swedish {

class PuzzleView::PuzzlePaintedWidget final : public Wt::WPaintedWidget {
public:
  PuzzlePaintedWidget(PuzzleView *puzzleView);
  virtual ~PuzzlePaintedWidget() override;

  void zoomIn();
  void zoomOut();

protected:
  virtual void paintEvent(Wt::WPaintDevice *paintDevice) override;

private:
  PuzzleView *puzzleView_;
  double zoom_;
};

PuzzleView::PuzzlePaintedWidget::PuzzlePaintedWidget(PuzzleView *puzzleView)
  : puzzleView_(puzzleView),
    zoom_(1.0)
{
  auto puzzle = puzzleView->puzzle_;
  resize(puzzle->width, puzzle->height);
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

  painter.scale(zoom_, zoom_);

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

void PuzzleView::PuzzlePaintedWidget::zoomIn()
{
  zoom_ = std::min(zoom_ + 0.1, max_zoom);

  resize(puzzleView_->puzzle_->width * zoom_,
         puzzleView_->puzzle_->height * zoom_);

  update();
}

void PuzzleView::PuzzlePaintedWidget::zoomOut()
{
  zoom_ = std::max(zoom_ - 0.1, min_zoom);

  resize(puzzleView_->puzzle_->width * zoom_,
         puzzleView_->puzzle_->height * zoom_);

  update();
}

PuzzleView::PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle)
  : Wt::WCompositeWidget(std::make_unique<Wt::WContainerWidget>()),
    puzzle_(puzzle),
    paintedWidget_(impl()->addNew<PuzzlePaintedWidget>(this)),
    zoomInBtn_(impl()->addNew<Wt::WPushButton>()),
    zoomOutBtn_(impl()->addNew<Wt::WPushButton>())
{
  Wt::WApplication *app = Wt::WApplication::instance();
  app->useStyleSheet(Wt::WApplication::relativeResourcesUrl() +
                     "font-awesome/css/font-awesome.min.css");

  impl()->setPositionScheme(Wt::PositionScheme::Relative);
  impl()->setOverflow(Wt::Overflow::Auto);

  zoomInBtn_->setPositionScheme(Wt::PositionScheme::Fixed);
  zoomOutBtn_->setPositionScheme(Wt::PositionScheme::Fixed);

  zoomInBtn_->setOffsets(10, Wt::Side::Top | Wt::Side::Left);
  zoomOutBtn_->setOffsets(10, Wt::Side::Left);
  zoomOutBtn_->setOffsets(46, Wt::Side::Top);

  zoomInBtn_->setTextFormat(Wt::TextFormat::XHTML);
  zoomOutBtn_->setTextFormat(Wt::TextFormat::XHTML);

  zoomInBtn_->setText(Wt::utf8("<i class=\"fa fa-search-plus\"></i>"));
  zoomOutBtn_->setText(Wt::utf8("<i class=\"fa fa-search-minus\"></i>"));

  zoomInBtn_->clicked().connect([this]{
    paintedWidget_->zoomIn();
  });
  zoomOutBtn_->clicked().connect([this]{
    paintedWidget_->zoomOut();
  });

  // TODO(Roel): zoom controls
  // TODO(Roel): other controls
  // TODO(Roel): clicked
}

PuzzleView::~PuzzleView()
{ }

Wt::WContainerWidget *PuzzleView::impl()
{
  return static_cast<Wt::WContainerWidget *>(implementation());
}

}
