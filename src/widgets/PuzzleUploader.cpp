#include "PuzzleUploader.h"

#include <Wt/WApplication.h>
#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WFileUpload.h>
#include <Wt/WImage.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WPushButton.h>
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

class PuzzleUploader::View : public Wt::WDialog {
public:
  View(PuzzleUploader *uploader, const Wt::WString &title);
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

PuzzleUploader::View::View(PuzzleUploader *uploader,
                           const Wt::WString &title)
  : WDialog(title),
    uploader_(uploader)
{
  setClosable(true);
}

PuzzleUploader::View::~View()
{ }

PuzzleUploader::UploadView::UploadView(PuzzleUploader *uploader)
  : View(uploader, Wt::utf8("Upload Swedish puzzle"))
{
  auto nextBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("next"));
  nextBtn->clicked().connect([this]{
    uploader_->state_ = State::SelectCell;
    done(Wt::DialogCode::Accepted);
  });
}

PuzzleUploader::UploadView::~UploadView()
{ }

PuzzleUploader::SelectCellView::SelectCellView(PuzzleUploader *uploader)
  : View(uploader, Wt::utf8("Configure puzzle"))
{
  resize(Wt::WLength(90, Wt::LengthUnit::ViewportWidth),
         Wt::WLength(90, Wt::LengthUnit::ViewportHeight));
  setResizable(true);

  auto nextBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("next"));
  nextBtn->clicked().connect([this]{
    uploader_->state_ = State::Processing;
    done(Wt::DialogCode::Accepted);
  });
}

PuzzleUploader::SelectCellView::~SelectCellView()
{ }

PuzzleUploader::ProcessingView::ProcessingView(PuzzleUploader *uploader)
  : View(uploader, Wt::utf8("Processing"))
{
  auto nextBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("next"));
  nextBtn->clicked().connect([this]{
    uploader_->state_ = State::Confirmation;
    done(Wt::DialogCode::Accepted);
  });
  auto cancelBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("cancel"));
  cancelBtn->clicked().connect([this]{
    done(Wt::DialogCode::Rejected);
  });
}

PuzzleUploader::ProcessingView::~ProcessingView()
{ }

PuzzleUploader::ConfirmationView::ConfirmationView(PuzzleUploader *uploader)
  : View(uploader, Wt::utf8("Confirm"))
{
  resize(Wt::WLength(90, Wt::LengthUnit::ViewportWidth),
         Wt::WLength(90, Wt::LengthUnit::ViewportHeight));
  setResizable(true);

  auto nextBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("next"));
  nextBtn->clicked().connect([this]{
    uploader_->state_ = State::Done;
    done(Wt::DialogCode::Accepted);
  });
  auto selectCellBtn = contents()->addNew<Wt::WPushButton>(Wt::utf8("go back to select cell"));
  selectCellBtn->clicked().connect([this]{
    uploader_->state_ = State::SelectCell;
    done(Wt::DialogCode::Accepted);
  });
}

PuzzleUploader::ConfirmationView::~ConfirmationView()
{ }

PuzzleUploader::PuzzleUploader()
{
  createStateView();
}

PuzzleUploader::~PuzzleUploader()
{ }

void PuzzleUploader::createStateView()
{
  if (state_ == State::Upload) {
    view_ = std::make_unique<UploadView>(this);
  } else if (state_ == State::SelectCell) {
    view_ = std::make_unique<SelectCellView>(this);
  } else if (state_ == State::Processing) {
    view_ = std::make_unique<ProcessingView>(this);
  } else {
    assert(state_ == State::Confirmation);
    view_ = std::make_unique<ConfirmationView>(this);
  }
  view_->finished().connect([this](Wt::DialogCode code) {
    if (code == Wt::DialogCode::Rejected ||
        state_ == State::Done) {
      done_.emit();
    } else {
      createStateView();
    }
  });
  view_->show();
}

}
