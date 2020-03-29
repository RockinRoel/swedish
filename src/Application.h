#ifndef SWEDISH_APPLICATION_H_
#define SWEDISH_APPLICATION_H_

#include <Wt/WApplication.h>

#include "Dispatcher.h"
#include "GlobalSession.h"
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
              GlobalSession *globalSession,
              Dispatcher *dispatcher);

  virtual ~Application() override;

  virtual void initialize() override;
  virtual void finalize() override;

  long long user() const { return user_; }
  const std::vector<UserCopy> &users() { return users_; }
  GlobalSession *globalSession() { return globalSession_; }
  Dispatcher *dispatcher() { return dispatcher_; }
  Subscriber *subscriber() { return &subscriber_; }

  static Application *instance() { return dynamic_cast<Application *>(Wt::WApplication::instance()); }

private:
  Session session_;
  GlobalSession *globalSession_;
  Dispatcher *dispatcher_;
  Subscriber subscriber_;
  Wt::WHBoxLayout *layout_;
  Wt::WVBoxLayout *rightLayout_;
  Wt::WContainerWidget *left_;
  Wt::WContainerWidget *userList_;
  long long user_;
  std::vector<UserCopy> users_;
  PuzzleView *puzzleView_;
  PuzzleUploader *puzzleUploader_;

  std::unique_ptr<Wt::WPanel> createChangeColorPanel(const Wt::WColor &color);

  void handleUserAdded(long long id,
                       const Wt::WString &name,
                       const Wt::WColor &color);

  void handleUserChangedColor(long long id,
                              const Wt::WColor &color);

  void handleCellValueChanged(long long puzzleId,
                              std::pair<int, int> cellRef);
};

}

#endif // SWEDISH_APPLICATION_H_
