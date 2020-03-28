#ifndef SWEDISH_PUZZLEUPLOADER_H_
#define SWEDISH_PUZZLEUPLOADER_H_

#include "../Rotation.h"

#include <Wt/WCompositeWidget.h>

namespace swedish {

class PuzzleUploader final : public Wt::WCompositeWidget {
public:
  PuzzleUploader();
  virtual ~PuzzleUploader() override;

private:
  class View;
  class UploadView;
  class SelectCellView;
  class ProcessingView;
  class ConfirmationView;

  View *view_ = nullptr;

  Wt::WStackedWidget *impl();
  void handleClicked(const Wt::WMouseEvent &evt);
  std::shared_ptr<Wt::WRasterImage> createImage(Rotation rotation) const;
  static std::shared_ptr<Wt::WRasterImage> fillImage(std::shared_ptr<Wt::WRasterImage> image,
                                                     const Wt::WPointF &point);
};

}

#endif // SWEDISH_PUZZLEUPLOADER_H_
