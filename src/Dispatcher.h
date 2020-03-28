#ifndef SWEDISH_DISPATCHER_H_
#define SWEDISH_DISPATCHER_H_

#include <Wt/WSignal.h>

#include "model/Puzzle.h"

#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <Wt/WColor.h>

namespace swedish {

class Subscriber;

// one dispatcher in the entire program, to send events
class Dispatcher final {
public:
  Dispatcher(Wt::WServer *server);

  void addSubsriber(Subscriber *subscriber);
  void removeSubscriber(Subscriber *subscriber);

  void notifyUserAdded(Subscriber *self,
                       long long id,
                       const Wt::WString &name,
                       const Wt::WColor &color);

  void notifyUserChangedColor(Subscriber *self,
                              long long id,
                              const Wt::WColor &color);

  void notifyCellValueChanged(Subscriber *self,
                              long long puzzleId,
                              std::pair<int, int> cellRef);

private:
  Wt::WServer *server_;
  std::mutex subscriberMutex_;
  std::vector<Subscriber *> subscribers_;
};

// one subsciber per WApplication, to receive events
class Subscriber final {
public:
  Subscriber(const std::string &sessionId);

  const std::string & sessionId() const { return sessionId_; }

  Wt::Signal<long long, const Wt::WString &, const Wt::WColor &> &userAdded() { return userAdded_; }
  Wt::Signal<long long, const Wt::WColor &> &userChangedColor() { return userChangedColor_; }
  Wt::Signal<long long, std::pair<int, int>> &cellValueChanged() { return cellValueChanged_; }

private:
  std::string sessionId_;
  Wt::Signal<long long, const Wt::WString &, const Wt::WColor &> userAdded_;
  Wt::Signal<long long, const Wt::WColor &> userChangedColor_;
  Wt::Signal<long long, std::pair<int, int>> cellValueChanged_;
};

}

#endif // SWEDISH_DISPATCHER_H_
