#include "PuzzleUploader.h"

#include <Wt/WApplication.h>
#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFileUpload.h>
#include <Wt/WImage.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WRasterImage.h>
#include <Wt/WRectF.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WTransform.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

namespace swedish {

class PuzzleUploader::View : public Wt::WContainerWidget {
public:
  View(PuzzleUploader *uploader);
  virtual ~View() override;

protected:
  PuzzleUploader *uploader_;
};

class PuzzleUploader::UploadView final : public View {
public:
  UploadView(PuzzleUploader *uploader);
  virtual ~UploadView() override;

private:
  Wt::WFileUpload *fileUpload_;
};

class PuzzleUploader::SelectCellView final : public View {
public:
  SelectCellView(PuzzleUploader *uploader);
  virtual ~SelectCellView() override;
};

class PuzzleUploader::ProcessingView final : public View {
public:
  ProcessingView(PuzzleUploader *uploader);
  virtual ~ProcessingView() override;
};

class PuzzleUploader::ConfirmationView final : public View {
public:
  ConfirmationView(PuzzleUploader *uploader);
  virtual ~ConfirmationView() override;
};

PuzzleUploader::View::View(PuzzleUploader *uploader)
  : uploader_(uploader)
{
}

PuzzleUploader::View::~View()
{ }

PuzzleUploader::UploadView::UploadView(PuzzleUploader *uploader)
  : View(uploader),
    fileUpload_(addNew<Wt::WFileUpload>())
{ }

PuzzleUploader::UploadView::~UploadView()
{ }

PuzzleUploader::SelectCellView::SelectCellView(PuzzleUploader *uploader)
  : View(uploader)
{ }

PuzzleUploader::SelectCellView::~SelectCellView()
{ }

PuzzleUploader::ProcessingView::ProcessingView(PuzzleUploader *uploader)
  : View(uploader)
{ }

PuzzleUploader::ProcessingView::~ProcessingView()
{ }

PuzzleUploader::ConfirmationView::ConfirmationView(PuzzleUploader *uploader)
  : View(uploader)
{ }

PuzzleUploader::ConfirmationView::~ConfirmationView()
{ }

PuzzleUploader::PuzzleUploader()
  : Wt::WCompositeWidget(std::make_unique<Wt::WStackedWidget>())
{
  Wt::WAnimation animation(Wt::AnimationEffect::SlideInFromRight,
                           Wt::TimingFunction::EaseInOut);
  impl()->setTransitionAnimation(animation, true);

  impl()->addNew<UploadView>(this);
}

PuzzleUploader::~PuzzleUploader()
{ }

Wt::WStackedWidget *PuzzleUploader::impl()
{
  return static_cast<Wt::WStackedWidget*>(implementation());
}

}
