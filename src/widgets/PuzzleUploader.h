#ifndef SWEDISH_PUZZLEUPLOADER_H_
#define SWEDISH_PUZZLEUPLOADER_H_

#include "../Rotation.h"

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <memory>

namespace swedish {

class PuzzleUploader final : public Wt::WObject {
public:
  PuzzleUploader();
  virtual ~PuzzleUploader() override;

  Wt::Signal<> &done() { return done_; }

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
  Wt::Signal<> done_;
  State state_ = State::Upload;

  void createStateView();
};

}

#endif // SWEDISH_PUZZLEUPLOADER_H_
