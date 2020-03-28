#ifndef SWEDISH_PUZZLEVIEW_H_
#define SWEDISH_PUZZLEVIEW_H_

#include "../Direction.h"
#include "../GlobalSession.h"

#include "../model/Puzzle.h"

#include <Wt/WCompositeWidget.h>

#include <array>

namespace swedish {

enum class PuzzleViewType {
  SelectCell,
  ViewCells,
  SolvePuzzle
};

class PuzzleView final : public Wt::WCompositeWidget {
public:
  PuzzleView(const Wt::Dbo::ptr<Puzzle> &puzzle,
             PuzzleViewType type);
  virtual ~PuzzleView() override;

private:
  class Layer;
  class PuzzlePaintedWidget;
  class TextLayer;

  Wt::Dbo::ptr<Puzzle> puzzle_;
  PuzzlePaintedWidget *paintedWidget_ = nullptr;
  TextLayer *textLayer_ = nullptr;
  std::pair<int, int> selectedCell_ = { -1, -1 };

  Wt::WPushButton *zoomInBtn_ = nullptr, *zoomOutBtn_ = nullptr;
  double zoom_ = 1.0;
  PuzzleViewType type_;
  Wt::Orientation direction_ = Wt::Orientation::Horizontal;

  Wt::WContainerWidget *impl();
  void zoomIn();
  void zoomOut();
  void handleClick(const Wt::WMouseEvent &evt);
  void handleKeyWentDown(const Wt::WKeyEvent &evt);
  void handleCellValueChanged(long long puzzleId, std::pair<int, int> cellRef);
  std::pair<int, int> nextCell(std::pair<int, int> cellRef,
                               Direction direction) const;
  std::pair<int, int> immediateNextCell(std::pair<int, int> cellRef,
                                        Direction direction) const;
};

}

#endif // SWEDISH_PUZZLEVIEW_H_
