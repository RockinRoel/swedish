// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#include "Application.h"

#include <Wt/WBootstrapTheme.h>
#include <Wt/WBreak.h>
#include <Wt/WColorPicker.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WDialog.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLabel.h>
#include <Wt/WLength.h>
#include <Wt/WLineEdit.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPanel.h>
#include <Wt/WPushButton.h>
#include <Wt/WRectF.h>
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

#include <Wt/Dbo/Dbo.h>

#include "model/Puzzle.h"
#include "model/User.h"
#include "widgets/PuzzleView.h"
#include "widgets/PuzzleUploader.h"

#include <memory>
#include <string>

namespace swedish {

Application::Application(const Wt::WEnvironment &env,
                         Wt::Dbo::SqlConnectionPool &pool,
                         SharedSession &sharedSession,
                         Dispatcher &dispatcher)
  : WApplication(env),
    session_(pool),
    sharedSession_(sharedSession),
    dispatcher_(dispatcher),
    subscriber_(sessionId()),
    layout_(nullptr),
    rightLayout_(nullptr),
    left_(nullptr),
    userList_(nullptr),
    puzzleView_(nullptr),
    puzzleUploader_(nullptr)
{
  enableUpdates();

  subscriber_.userAdded().connect(this, &Application::handleUserAdded);
  subscriber_.userChangedColor().connect(this, &Application::handleUserChangedColor);

  messageResourceBundle().use(appRoot() + "template");
  messageResourceBundle().use(appRoot() + "strings");
  useStyleSheet("/css/style.css");

  auto theme = std::make_shared<Wt::WBootstrapTheme>();
  theme->setVersion(Wt::BootstrapVersion::v3);
  setTheme(theme);

  layout_ = root()->setLayout(std::make_unique<Wt::WHBoxLayout>());
  layout_->setContentsMargins(3, 3, 3, 3);
  layout_->setSpacing(3);

  left_ = layout_->addWidget(std::make_unique<Wt::WContainerWidget>(), 0);
  left_->setWidth(Wt::WLength(300, Wt::LengthUnit::Pixel));

  auto instructionsPanel = left_->addNew<Wt::WPanel>();
  instructionsPanel->setTitle(Wt::utf8("Instructions"));
  instructionsPanel->setCentralWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("str.swedish.instructions")));

  auto usersPanel = left_->addNew<Wt::WPanel>();
  usersPanel->setTitle(Wt::utf8("Users"));

  userList_ = usersPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  userList_->setList(true);

  {
    Wt::Dbo::Transaction t(session_);

    {
      Wt::Dbo::collection<Wt::Dbo::ptr<User>> users = session_.find<User>().orderBy("id");

      for (const auto &user: users) {
        users_.push_back({user.id(), user->name, user->color});
      }
    }
  }

  Wt::WFont font;
  font.setFamily(Wt::FontFamily::SansSerif);
  font.setSize(16);
  font.setWeight(Wt::FontWeight::Bold);
  for (auto&& user : users_) {
    auto c = userList_->addNew<Wt::WContainerWidget>();
    c->addNew<Wt::WText>(user.name, Wt::TextFormat::Plain);
    c->decorationStyle().setFont(font);
    c->decorationStyle().setForegroundColor(user.color);
  }

  rightLayout_ = layout_->addLayout(std::make_unique<Wt::WVBoxLayout>(), 1);
  rightLayout_->setContentsMargins(3, 3, 3, 3);
  rightLayout_->setSpacing(3);

  auto topbar = rightLayout_->addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("tpl.swedish.topbar")));

  auto prevBtn = topbar->bindNew<Wt::WPushButton>("prev-btn", Wt::utf8("Previous"));
  auto puzzleUploadBtn = topbar->bindNew<Wt::WPushButton>("upload-btn", Wt::utf8("Upload new puzzle"));
  puzzleEdit_ = topbar->bindNew<Wt::WLineEdit>("current-puzzle-edit");
  auto goBtn = topbar->bindNew<Wt::WPushButton>("go-btn", Wt::utf8("Go"));
  auto nextBtn = topbar->bindNew<Wt::WPushButton>("next-btn", Wt::utf8("Next"));

  prevBtn->clicked().connect([this]{
    const long long newPuzzle = currentPuzzle_ - 1;
    changePuzzle(newPuzzle);
  });
  nextBtn->clicked().connect([this]{
    const long long newPuzzle = currentPuzzle_ + 1;
    changePuzzle(newPuzzle);
  });
  auto puzzleEditReturnOrGoClick = [this]{
    const std::string s = puzzleEdit_->valueText().toUTF8();

    std::string::size_type sz = 0;
    try {
      const long long newPuzzle = std::stoll(s, &sz, 10);
      if (sz == s.size()) {
        changePuzzle(newPuzzle);
      }
    } catch (std::invalid_argument&) { }
  };
  puzzleEdit_->enterPressed().connect(puzzleEditReturnOrGoClick);
  goBtn->clicked().connect(puzzleEditReturnOrGoClick);

  puzzleContainer_ = rightLayout_->addWidget(std::make_unique<Wt::WContainerWidget>(), 1);

  long long puzzleId = -1;
  const std::string path = internalPath();
  if (path.size() > 1 &&
      path[0] == '/') {
    std::string::size_type sz = 0;
    try {
      puzzleId = std::stoll(path.substr(1), &sz, 10);
      if (sz + 1 != path.size()) {
        puzzleId = -1;
      }
    } catch (std::invalid_argument &) { }
  }
  if (puzzleId == -1) {
    Wt::Dbo::Transaction t(session_);

    auto puzzle = session_.find<Puzzle>().orderBy("id desc").limit(1).resultValue();
    if (puzzle) {
      puzzleId = puzzle.id();
    }
  }

  changePuzzle(puzzleId);

  auto chooseUserDialog = addChild(std::make_unique<Wt::WDialog>(Wt::utf8("Choose user")));

  for (auto&& user : users_) {
    auto btn = chooseUserDialog->contents()->addNew<Wt::WPushButton>();
    btn->setTextFormat(Wt::TextFormat::Plain);
    btn->setText(user.name);
    btn->decorationStyle().setForegroundColor(user.color);
    btn->decorationStyle().setFont(font);
    btn->clicked().connect([this, id=user.id, color=user.color, chooseUserDialog]{
      user_ = id;
      left_->addWidget(createChangeColorPanel(color));
      chooseUserDialog->done(Wt::DialogCode::Accepted);
    });
  }

  chooseUserDialog->contents()->addNew<Wt::WBreak>();

  auto newUserLabel = chooseUserDialog->contents()->addNew<Wt::WLabel>(Wt::utf8("New user: "));
  auto newUserName = chooseUserDialog->contents()->addNew<Wt::WLineEdit>();
  newUserLabel->setBuddy(newUserName);
  auto newUserColor = chooseUserDialog->contents()->addNew<Wt::WColorPicker>();
  auto newUserButton = chooseUserDialog->contents()->addNew<Wt::WPushButton>(Wt::utf8("Create"));

  newUserButton->clicked().connect([this,newUserName,newUserColor,chooseUserDialog,font]{
    Wt::WString name = newUserName->valueText();
    Wt::WColor color = newUserColor->color();
    {
      Wt::Dbo::Transaction t(session_);

      auto user = std::make_unique<User>();
      user->name = name;
      user->color = color;
      auto userPtr = session_.add(std::move(user));
      session_.flush();
      user_ = userPtr.id();
    }
    dispatcher_.get().notifyUserAdded(subscriber_, user_, name, color);
    handleUserAdded(user_, name, color);
    left_->addWidget(createChangeColorPanel(color));
    chooseUserDialog->done(Wt::DialogCode::Accepted);
  });

  chooseUserDialog->finished().connect([this, chooseUserDialog]{
    removeChild(chooseUserDialog);
  });

  chooseUserDialog->show();

  puzzleUploadBtn->clicked().connect([this]{
    puzzleUploader_ = addChild(std::make_unique<PuzzleUploader>());
    puzzleUploader_->done().connect([this](Wt::DialogCode code){
      if (code == Wt::DialogCode::Accepted) {
        {
          Wt::Dbo::Transaction t(session_);

          Wt::Dbo::ptr<Puzzle> puzzle = puzzleUploader_->puzzle();
          session_.add(puzzle);

          session_.flush();

          changePuzzle(puzzle.id());
        }
      }
      removeChild(puzzleUploader_);
      puzzleUploader_ = nullptr;
    });
  });
}

Application::~Application()
{
  if (puzzleUploader_) {
    removeChild(puzzleUploader_);
  }
}

void Application::initialize()
{
  dispatcher_.get().addSubsriber(subscriber_);
}

void Application::finalize()
{
  if (user_ != -1) {
    dispatcher_.get().notifyCursorMoved(subscriber_,
                                        -1,
                                        user_,
                                        std::pair(-1, -1),
                                        Wt::Orientation::Horizontal);
  }

  dispatcher_.get().removeSubscriber(subscriber_);
}

std::unique_ptr<Wt::WPanel> Application::createChangeColorPanel(const Wt::WColor &color)
{
  auto changeColorPanel = std::make_unique<Wt::WPanel>();
  changeColorPanel->setTitle(Wt::utf8("Change color"));

  auto changeColorContainer = changeColorPanel->setCentralWidget(std::make_unique<Wt::WContainerWidget>());
  auto colorPicker = changeColorContainer->addNew<Wt::WColorPicker>(color);
  auto changeColorButton = changeColorContainer->addNew<Wt::WPushButton>(Wt::utf8("Update"));

  changeColorButton->clicked().connect([this, colorPicker]{
    Wt::WColor color = colorPicker->color();

    {
      Wt::Dbo::Transaction t(session_);

      auto user = session_.load<User>(user_);
      user.modify()->color = color;
    }

    dispatcher_.get().notifyUserChangedColor(subscriber_, user_, color);

    handleUserChangedColor(user_, color);
  });

  return changeColorPanel;
}

void Application::handleUserAdded(long long id,
                                  const Wt::WString &name,
                                  const Wt::WColor &color)
{
  users_.push_back({id, name, color});

  Wt::WFont font;
  font.setFamily(Wt::FontFamily::SansSerif);
  font.setSize(16);
  font.setWeight(Wt::FontWeight::Bold);
  auto c = userList_->addNew<Wt::WContainerWidget>();
  c->addNew<Wt::WText>(name, Wt::TextFormat::Plain);
  c->decorationStyle().setFont(font);
  c->decorationStyle().setForegroundColor(color);

  triggerUpdate();
}

void Application::handleUserChangedColor(long long id,
                                         const Wt::WColor &color)
{
  auto it = std::find_if(begin(users_), end(users_), [id](const UserCopy &user){
    return user.id == id;
  });
  if (it == end(users_))
    return; // TODO(Roel): error!

  it->color = color;

  int userIdx = static_cast<int>(std::distance(begin(users_), it));
  auto w = userList_->widget(userIdx);
  w->decorationStyle().setForegroundColor(color);

  if (puzzleView_) {
    puzzleView_->update();
  }

  triggerUpdate();
}

void Application::changePuzzle(long long id)
{
  Wt::Dbo::Transaction t(session_);

  Wt::Dbo::ptr<Puzzle> puzzle;
  try {
    puzzle = session_.load<Puzzle>(id);
  } catch (Wt::Dbo::Exception &) { }

  if (!puzzle) {
    if (currentPuzzle_ == -1) {
      setInternalPath("/");
      puzzleEdit_->setText(Wt::WString::Empty);
    } else {
      puzzleEdit_->setText(Wt::utf8("{1}").arg(currentPuzzle_));
    }
    return;
  } else {
    puzzleEdit_->setText(Wt::utf8("{1}").arg(puzzle.id()));
  }

  currentPuzzle_ = id;
  puzzleContainer_->clear();
  puzzleView_ = nullptr;
  puzzleView_ = puzzleContainer_->addNew<PuzzleView>(puzzle, PuzzleViewType::SolvePuzzle);
  puzzleView_->resize(Wt::WLength(100, Wt::LengthUnit::Percentage),
                      Wt::WLength(100, Wt::LengthUnit::Percentage));
  setInternalPath("/" + std::to_string(currentPuzzle_));
}

}
