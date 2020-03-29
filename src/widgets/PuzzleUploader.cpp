#include "PuzzleUploader.h"

#include "PuzzleView.h"

#include "../jobs/SquareFinder.h"

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
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WRasterImage.h>
#include <Wt/WRectF.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WTransform.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

namespace {

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

}

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
  : View(uploader, Wt::utf8("Upload Swedish puzzle")),
    fileUpload_(contents()->addNew<Wt::WFileUpload>())
{
  fileUpload_->setProgressBar(std::make_unique<Wt::WProgressBar>());
  fileUpload_->changed().connect([this]{
    fileUpload_->upload();
  });
  fileUpload_->fileTooLarge().connect([this]{
    // TODO(Roel): handle this?
    std::cout << "FILE TOO LARGE!\n";
    std::cout << "MAX SIZE: " << Wt::WApplication::instance()->maximumRequestSize() << '\n';
  });
  fileUpload_->setFilters("image/jpeg");
  fileUpload_->uploaded().connect([this]{
    const auto &files = fileUpload_->uploadedFiles();
    const std::string &fileName = files[0].spoolFileName();
    files[0].stealSpoolFile();
    boost::filesystem::path path = fileName;
    path.replace_extension(".jpg");
    boost::filesystem::path docRoot = Wt::WApplication::instance()->docRoot();
    boost::filesystem::copy(fileName, docRoot / "puzzles" / path.filename());
    boost::filesystem::remove(fileName);

    uploader_->puzzle_ = Wt::Dbo::make_ptr<Puzzle>();
    std::string uploadedFileName = ("puzzles" / path.filename()).string();
    uploader_->puzzle_.modify()->path = uploadedFileName;
    {
      const Wt::WApplication *app = Wt::WApplication::instance();
      boost::filesystem::path docRoot = app->docRoot();
      Wt::WPainter::Image img((docRoot / uploadedFileName).string(),
                              (docRoot / uploadedFileName).string());
      uploader_->puzzle_.modify()->width = img.width();
      uploader_->puzzle_.modify()->height = img.height();
    }
    uploader_->puzzle_.modify()->rotation = Rotation::None;

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

  auto confirmBtn = footer()->addNew<Wt::WPushButton>(Wt::utf8("Confirm"));
  confirmBtn->setDisabled(true);
  confirmBtn->clicked().connect([this]{
    uploader_->state_ = State::Processing;
    done(Wt::DialogCode::Accepted);
  });

  auto puzzleView = contents()->addNew<PuzzleView>(uploader_->puzzle_, PuzzleViewType::SelectCell);
  puzzleView->clickPositionChanged().connect([this, confirmBtn](Wt::WPointF point) {
    uploader_->clickedPoint_ = point;
    confirmBtn->setDisabled(false);
  });

  if (uploader_->clickedPoint_) {
    puzzleView->setClickedPoint(uploader_->clickedPoint_.value());
    confirmBtn->setDisabled(false);
  }
}

PuzzleUploader::SelectCellView::~SelectCellView()
{ }

PuzzleUploader::ProcessingView::ProcessingView(PuzzleUploader *uploader)
  : View(uploader, Wt::utf8("Processing"))
{
  auto squareFinder = addChild(std::make_unique<SquareFinder>(*uploader_->puzzle_.modify(),
                                                              static_cast<int>(uploader_->clickedPoint_->x()),
                                                              static_cast<int>(uploader_->clickedPoint_->y())));

  auto statusLabel = contents()->addNew<Wt::WText>();
  statusLabel->setTextFormat(Wt::TextFormat::Plain);

  squareFinder->statusChanged().connect([this, statusLabel](SquareFinder::Status status) {
    std::visit(overload{
                 [statusLabel](SquareFinder::ReadingImage &) {
                   statusLabel->setText(Wt::utf8("Reading image data..."));
                 },
                 [statusLabel](SquareFinder::Processing &processing) {
                   statusLabel->setText(Wt::utf8("Processing... (queue length: {1})").arg(processing.queueSize));
                 },
                 [statusLabel](SquareFinder::PopulatingPuzzle &) {
                   statusLabel->setText(Wt::utf8("Populating puzzle..."));
                 },
                 [this](SquareFinder::Done &) {
                   uploader_->state_ = State::Confirmation;
                   done(Wt::DialogCode::Accepted);
                 }
               }, status);
  });

  squareFinder->start();

  auto cancelBtn = footer()->addNew<Wt::WPushButton>(Wt::utf8("Cancel"));
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

  contents()->addNew<PuzzleView>(uploader_->puzzle_, PuzzleViewType::ViewCells);

  auto selectCellBtn = footer()->addNew<Wt::WPushButton>(Wt::utf8("Go back to select cell"));
  selectCellBtn->clicked().connect([this]{
    uploader_->state_ = State::SelectCell;
    done(Wt::DialogCode::Accepted);
  });
  auto confirmBtn = footer()->addNew<Wt::WPushButton>(Wt::utf8("Confirm"));
  confirmBtn->clicked().connect([this]{
    uploader_->state_ = State::Done;
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
      done_(code);
    } else {
      createStateView();
    }
  });
  view_->show();
}

}
