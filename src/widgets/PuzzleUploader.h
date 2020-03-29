#ifndef SWEDISH_PUZZLEUPLOADER_H_
#define SWEDISH_PUZZLEUPLOADER_H_

#include "../model/Puzzle.h"
#include "../Rotation.h"

#include <Wt/WDialog.h>
#include <Wt/WObject.h>
#include <Wt/WPointF.h>
#include <Wt/WSignal.h>

#include <Wt/Dbo/Dbo.h>

#include <memory>
#include <optional>

namespace swedish {

class PuzzleUploader final : public Wt::WObject {
public:
  PuzzleUploader();
  virtual ~PuzzleUploader() override;

  Wt::Signal<Wt::DialogCode> &done() { return done_; }

  Wt::Dbo::ptr<Puzzle> & puzzle() { return puzzle_; }

private:
  enum class State {
    Upload,
    SelectCell,
    Processing,
    Confirmation,
    Done
  };

  class View;
  class UploadView;
  class SelectCellView;
  class ProcessingView;
  class ConfirmationView;

  std::unique_ptr<View> view_;
  Wt::Dbo::ptr<Puzzle> puzzle_;
  std::optional<Wt::WPointF> clickedPoint_;
  Wt::Signal<Wt::DialogCode> done_;
  State state_ = State::Upload;

  void createStateView();
};

}

#endif // SWEDISH_PUZZLEUPLOADER_H_
