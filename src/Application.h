#ifndef SWEDISH_APPLICATION_H_
#define SWEDISH_APPLICATION_H_

#include <Wt/WApplication.h>

#include "Dispatcher.h"
#include "SharedSession.h"
#include "UserCopy.h"

#include "model/User.h"
#include "model/Session.h"

#include <vector>

namespace swedish {

class PuzzleUploader;
class PuzzleView;

class Application final : public Wt::WApplication {
public:
  Application(const Wt::WEnvironment &env,
              Wt::Dbo::SqlConnectionPool &pool,
              SharedSession *sharedSession,
              Dispatcher *dispatcher);

  virtual ~Application() override;

  virtual void initialize() override;
  virtual void finalize() override;

  long long user() const { return user_; }
  const std::vector<UserCopy> &users() { return users_; }
  SharedSession *sharedSession() { return sharedSession_; }
  Dispatcher *dispatcher() { return dispatcher_; }
  Subscriber *subscriber() { return &subscriber_; }

  static Application *instance() { return dynamic_cast<Application *>(Wt::WApplication::instance()); }

private:
  Session session_;
  SharedSession *sharedSession_;
  Dispatcher *dispatcher_;
  Subscriber subscriber_;
  Wt::WHBoxLayout *layout_;
  Wt::WVBoxLayout *rightLayout_;
  Wt::WContainerWidget *left_;
  Wt::WContainerWidget *userList_;
  long long user_ = -1;
  std::vector<UserCopy> users_;
  long long currentPuzzle_ = -1;
  Wt::WContainerWidget *puzzleContainer_ = nullptr;
  PuzzleView *puzzleView_  = nullptr;
  PuzzleUploader *puzzleUploader_ = nullptr;
  Wt::WLineEdit *puzzleEdit_ = nullptr;

  std::unique_ptr<Wt::WPanel> createChangeColorPanel(const Wt::WColor &color);

  void handleUserAdded(long long id,
                       const Wt::WString &name,
                       const Wt::WColor &color);

  void handleUserChangedColor(long long id,
                              const Wt::WColor &color);

  void changePuzzle(long long id);
};

}

#endif // SWEDISH_APPLICATION_H_
