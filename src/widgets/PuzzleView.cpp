#include "PuzzleView.h"

#include "../Application.h"
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
#include <cmath>
#include <memory>
#include <utility>

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

  void setSelectedCell(const Cell *cell);

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

  const auto & puzzle = puzzleView_->puzzle_;
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

  for (std::size_t r = 0; r < puzzle()->rows_.size(); ++r) {
    const auto &row = puzzle()->rows_[r];
    for (std::size_t c = 0; c < row.size(); ++c) {
      const auto &cell = row[c];
      if (cell.isNull()) {
        continue;
      }

      const Wt::WRectF &square = cell.square;
      const double minSize = std::min(square.width(), square.height());

      if (std::make_pair(static_cast<int>(r), static_cast<int>(c)) == puzzleView_->selectedCell_) {
        painter.save();

        const Wt::WColor cellColor(150, 150, 150, 150);
        painter.setBrush(Wt::WBrush(cellColor));

        const Wt::WPointF center = square.center();
        const double diameter = minSize * 0.8;
        painter.drawEllipse(Wt::WRectF(center.x() - diameter / 2.0,
                                       center.y() - diameter / 2.0,
                                       diameter,
                                       diameter));

        painter.restore();
      }

      if (puzzleView_->type_ == PuzzleViewType::SolvePuzzle) {
        GlobalSession * const session = Application::instance()->globalSession();
        auto [ch, userId] = session->charAt(puzzle()->id(), { static_cast<int>(r), static_cast<int>(c) });
        // TODO(Roel): coloring depending on userid!

        if (ch != Character::None) {
          std::string str(charToStr(ch));

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
  }
}

PuzzleView::PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle,
                       PuzzleViewType type)
  : Wt::WCompositeWidget(std::make_unique<Wt::WContainerWidget>()),
    puzzle_(puzzle),
    type_(type)
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

  auto leftBtnGroup = top->addNew<Wt::WContainerWidget>();
  leftBtnGroup->addStyleClass("btn-group");
  zoomInBtn_ = leftBtnGroup->addNew<Wt::WPushButton>();
  zoomOutBtn_ = leftBtnGroup->addNew<Wt::WPushButton>();

  if (type_ == PuzzleViewType::SelectCell ||
      type_ == PuzzleViewType::SolvePuzzle) {
    auto rightBtnGroup = top->addNew<Wt::WContainerWidget>();
    rightBtnGroup->addStyleClass("btn-group");
    rightBtnGroup->addStyleClass("pull-right");

    if (type_ == PuzzleViewType::SelectCell) {
      auto rotateCWBtn = rightBtnGroup->addNew<Wt::WPushButton>();
      auto rotateCCWBtn = rightBtnGroup->addNew<Wt::WPushButton>();

      rotateCWBtn->setTextFormat(Wt::TextFormat::XHTML);
      rotateCCWBtn->setTextFormat(Wt::TextFormat::XHTML);

      rotateCWBtn->setText(Wt::utf8("<i class=\"fa fa-rotate-right\"></i> Rotate CW"));
      rotateCCWBtn->setText(Wt::utf8("<i class=\"fa fa-rotate-left\"></i> Rotate CCW"));
    } else {
      assert(type_ == PuzzleViewType::SolvePuzzle);
      auto horizontalBtn = rightBtnGroup->addNew<Wt::WPushButton>();
      auto verticalBtn = rightBtnGroup->addNew<Wt::WPushButton>();

      horizontalBtn->setTextFormat(Wt::TextFormat::XHTML);
      verticalBtn->setTextFormat(Wt::TextFormat::XHTML);

      horizontalBtn->addStyleClass("active");

      horizontalBtn->setText(Wt::utf8("<i class=\"fa fa-caret-right\"></i> Horizontal"));
      verticalBtn->setText(Wt::utf8("<i class=\"fa fa-caret-down\"></i> Vertical"));

      horizontalBtn->clicked().connect([this, horizontalBtn, verticalBtn]{
        if (direction_ == Wt::Orientation::Horizontal) {
          return;
        }
        direction_ = Wt::Orientation::Horizontal;
        horizontalBtn->addStyleClass("active");
        verticalBtn->removeStyleClass("active");
      });
      verticalBtn->clicked().connect([this, horizontalBtn, verticalBtn]{
        if (direction_ == Wt::Orientation::Vertical) {
          return;
        }
        direction_ = Wt::Orientation::Vertical;
        horizontalBtn->removeStyleClass("active");
        verticalBtn->addStyleClass("active");
      });

      app->globalKeyWentDown().connect(this, &PuzzleView::handleKeyWentDown);

      Application::instance()->subscriber()->cellValueChanged().connect(this, &PuzzleView::handleCellValueChanged);
    }

    bottom->clicked().connect(this, &PuzzleView::handleClick);
  }

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

void PuzzleView::handleClick(const Wt::WMouseEvent &evt)
{
  const Wt::Coordinates coords = evt.widget();

  const double x = coords.x / zoom_;
  const double y = coords.y / zoom_;

  if (type_ == PuzzleViewType::SolvePuzzle) {
    std::pair<int, int> closestCell = { -1,  -1};
    double smallestDistance = -1;
    for (std::size_t r = 0; r < puzzle_->rows_.size(); ++r) {
      const auto &row = puzzle_->rows_[r];
      for (std::size_t c = 0; c < row.size(); ++c) {
        const auto &cell = row[c];
        if (cell.square.contains(Wt::WPointF(x, y))) {
          const Wt::WPointF center = cell.square.center();
          const double distance = std::hypot(x - center.x(), y - center.y());
          if (closestCell == std::make_pair( -1, -1 ) ||
              distance < smallestDistance) {
            closestCell = std::make_pair(static_cast<int>(r),
                                         static_cast<int>(c));
            smallestDistance = distance;
          }
        }
      }
    }

    selectedCell_ = closestCell;

    textLayer_->update();
  } else {
    assert(type_ == PuzzleViewType::SelectCell);
    // TODO(Roel): square detection algorithm and stuff
  }
}

void PuzzleView::handleKeyWentDown(const Wt::WKeyEvent &evt)
{
  std::cout << "KEY WENT DOWN!\n";

  if (selectedCell_ == std::make_pair(-1, -1)) {
    return;
  }

  Application * const app = Application::instance();

  if (evt.key() == Wt::Key::Delete) {
    app->globalSession()->updateChar(puzzle_.id(),
                                     selectedCell_,
                                     Character::None,
                                     app->user());

    app->dispatcher()->notifyCellValueChanged(app->subscriber(),
                                              puzzle_.id(),
                                              selectedCell_);

    textLayer_->update();

    return;
  }

  if (evt.key() == Wt::Key::Backspace) {
    Character currentChar = app->globalSession()->charAt(puzzle_.id(), selectedCell_).first;

    app->globalSession()->updateChar(puzzle_.id(),
                                     selectedCell_,
                                     currentChar == Character::IJ ? Character::I : Character::None,
                                     app->user());

    app->dispatcher()->notifyCellValueChanged(app->subscriber(),
                                              puzzle_.id(),
                                              selectedCell_);

    if (currentChar != Character::IJ) {
      selectedCell_ = nextCell(selectedCell_, direction_ == Wt::Orientation::Horizontal ? Direction::Left : Direction::Up);
    }

    textLayer_->update();

    return;
  }

  if (evt.key() == Wt::Key::J) {
    const std::pair<int, int> previous = nextCell(selectedCell_, direction_ == Wt::Orientation::Horizontal ? Direction::Left : Direction::Up);
    if (previous != selectedCell_ &&
        app->globalSession()->charAt(puzzle_.id(), previous).first == Character::I) {
      app->globalSession()->updateChar(puzzle_.id(),
                                       previous,
                                       Character::IJ,
                                       app->user());

      app->dispatcher()->notifyCellValueChanged(app->subscriber(),
                                                puzzle_.id(),
                                                previous);

      textLayer_->update();

      return;
    }
    const std::pair<int, int> next = immediateNextCell(selectedCell_, direction_ == Wt::Orientation::Horizontal ? Direction::Right : Direction::Down);
    if (next == std::make_pair(-1, -1) &&
        app->globalSession()->charAt(puzzle_.id(), selectedCell_).first == Character::I) {
      app->globalSession()->updateChar(puzzle_.id(),
                                       selectedCell_,
                                       Character::IJ,
                                       app->user());

      app->dispatcher()->notifyCellValueChanged(app->subscriber(),
                                                puzzle_.id(),
                                                selectedCell_);

      selectedCell_ = nextCell(selectedCell_, direction_ == Wt::Orientation::Horizontal ? Direction::Right : Direction::Down);

      textLayer_->update();

      return;
    }
  }

  if (evt.key() == Wt::Key::Up) {
    selectedCell_ = nextCell(selectedCell_, Direction::Up);
    textLayer_->update();
    return;
  }
  if (evt.key() == Wt::Key::Right) {
    selectedCell_ = nextCell(selectedCell_, Direction::Right);
    textLayer_->update();
    return;
  }
  if (evt.key() == Wt::Key::Down) {
    selectedCell_ = nextCell(selectedCell_, Direction::Down);
    textLayer_->update();
    return;
  }
  if (evt.key() == Wt::Key::Left) {
    selectedCell_ = nextCell(selectedCell_, Direction::Left);
    textLayer_->update();
    return;
  }

  const Wt::Key key = evt.key();
  constexpr int a = static_cast<int>('A');
  constexpr int z = static_cast<int>('Z');
  const int keyI = static_cast<int>(key);
  if (keyI >= a && keyI <= z) {
    std::string s;
    s += static_cast<char>(keyI);

    app->globalSession()->updateChar(puzzle_.id(),
                                     selectedCell_,
                                     strToChar(s),
                                     app->user());

    app->dispatcher()->notifyCellValueChanged(app->subscriber(),
                                              puzzle_.id(),
                                              selectedCell_);

    selectedCell_ = nextCell(selectedCell_, direction_ == Wt::Orientation::Horizontal ? Direction::Right : Direction::Down);

    textLayer_->update();
  }
}

void PuzzleView::handleCellValueChanged(long long puzzleId,
                                        std::pair<int, int> cellRef)
{
  (void) cellRef;

  if (puzzleId != puzzle_->id()) {
    return;
  }

  textLayer_->update();

  Application::instance()->triggerUpdate();
}

std::pair<int, int> PuzzleView::nextCell(std::pair<int, int> cellRef,
                                         Direction direction) const
{
  if (cellRef == std::make_pair(-1, -1)) {
    return cellRef;
  }

  assert(cellRef.first >= 0 &&
         cellRef.second >= 0);

  const auto [r, c] = cellRef;

  const auto &row = puzzle_->rows_[static_cast<std::size_t>(r)];
  const int rowCount = static_cast<int>(puzzle_->rows_.size());
  const int colCount = static_cast<int>(row.size());

  if (direction == Direction::Up) {
    int nextRowUp = r - 1;
    while (nextRowUp >= 0 &&
           puzzle_->cell(nextRowUp, c).isNull()) {
      --nextRowUp;
    }
    if (nextRowUp < 0) {
      return cellRef;
    } else {
      return std::make_pair(nextRowUp, c);
    }
  } else if (direction == Direction::Right) {
    int nextColRight = c + 1;
    while (nextColRight < colCount &&
           puzzle_->cell(r, nextColRight).isNull()) {
      ++nextColRight;
    }
    if (nextColRight == colCount) {
      return cellRef;
    } else {
      return std::make_pair(r, nextColRight);
    }
  } else if (direction == Direction::Down) {
    int nextRowDown = r + 1;
    while (nextRowDown < rowCount &&
           puzzle_->cell(nextRowDown, c).isNull()) {
      ++nextRowDown;
    }
    if (nextRowDown == rowCount) {
      return cellRef;
    } else {
      return std::make_pair(nextRowDown, c);
    }
  } else {
    assert(direction == Direction::Left);
    int nextColLeft = c - 1;
    while (nextColLeft >= 0 &&
           puzzle_->cell(r, nextColLeft).isNull()) {
      --nextColLeft;
    }
    if (nextColLeft < 0) {
      return cellRef;
    } else {
      return std::make_pair(r, nextColLeft);
    }
  }
}

std::pair<int, int> PuzzleView::immediateNextCell(std::pair<int, int> cellRef,
                                                  Direction direction) const
{
  // For checking the previous cell, for 'ij'

  if (cellRef == std::make_pair(-1, -1)) {
    return cellRef;
  }

  assert(cellRef.first >= 0 &&
         cellRef.second >= 0);

  const auto [r, c] = cellRef;

  const auto &row = puzzle_->rows_[static_cast<std::size_t>(r)];
  const int rowCount = static_cast<int>(puzzle_->rows_.size());
  const int colCount = static_cast<int>(row.size());

  if (direction == Direction::Up) {
    const int nextRowUp = r - 1;
    if (nextRowUp >= 0 &&
        !puzzle_->cell(nextRowUp, c).isNull()) {
      return std::make_pair(nextRowUp, c);
    } else {
      return std::make_pair(-1, -1);
    }
  } else if (direction == Direction::Right) {
    const int nextColRight = c + 1;
    if (nextColRight < colCount &&
        !puzzle_->cell(r, nextColRight).isNull()) {
      return std::make_pair(r, nextColRight);
    } else {
      return std::make_pair(-1, -1);
    }
  } else if (direction == Direction::Down) {
    const int nextRowDown = r + 1;
    if (nextRowDown < rowCount &&
        !puzzle_->cell(nextRowDown, c).isNull()) {
      return std::make_pair(nextRowDown, c);
    } else {
      return std::make_pair(-1, -1);
    }
  } else {
    assert(direction == Direction::Left);
    const int nextColLeft = c - 1;
    if (nextColLeft >= 0 &&
        !puzzle_->cell(r, nextColLeft).isNull()) {
      return std::make_pair(r, nextColLeft);
    } else {
      return std::make_pair(-1, -1);
    }
  }
}

}
