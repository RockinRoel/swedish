#ifndef SWEDISH_PUZZLEVIEW_H_
#define SWEDISH_PUZZLEVIEW_H_

#include "../model/Puzzle.h"

#include <Wt/WCompositeWidget.h>

#include <array>

namespace swedish {

class PuzzleView final : public Wt::WCompositeWidget {
public:
  PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle);
  virtual ~PuzzleView() override;

private:
  class Layer;
  class PuzzlePaintedWidget;
  class TextLayer;

  Wt::Dbo::ptr<Puzzle> puzzle_;
  PuzzlePaintedWidget *paintedWidget_ = nullptr;
  TextLayer *textLayer_ = nullptr;

  Wt::WPushButton *zoomInBtn_ = nullptr, *zoomOutBtn_ = nullptr;
  double zoom_ = 1.0;

  Wt::WContainerWidget *impl();
  void zoomIn();
  void zoomOut();
};

}

#endif // SWEDISH_PUZZLEVIEW_H_
