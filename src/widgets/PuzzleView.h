#ifndef SWEDISH_PUZZLEVIEW_H_
#define SWEDISH_PUZZLEVIEW_H_

#include "../model/Puzzle.h"

#include <Wt/WCompositeWidget.h>

namespace swedish {

class PuzzleView final : public Wt::WCompositeWidget {
public:
  PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle);
  virtual ~PuzzleView() override;

private:
  class PuzzlePaintedWidget;

  Wt::Dbo::ptr<Puzzle> puzzle_;
  PuzzlePaintedWidget *paintedWidget_;

  Wt::WPushButton *zoomInBtn_, *zoomOutBtn_;

  Wt::WContainerWidget *impl();
};

}

#endif // SWEDISH_PUZZLEVIEW_H_
