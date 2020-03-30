#include "Dispatcher.h"

#include <Wt/WServer.h>

#include <algorithm>

namespace swedish {

Dispatcher::Dispatcher(Wt::WServer *server)
  : server_(server)
{ }

void Dispatcher::addSubsriber(Subscriber *subscriber)
{
  std::scoped_lock<std::mutex> lock(subscriberMutex_);
  subscribers_.push_back(subscriber);
}

void Dispatcher::removeSubscriber(Subscriber *subscriber)
{
  std::scoped_lock<std::mutex> lock(subscriberMutex_);

  auto it = std::find(begin(subscribers_), end(subscribers_), subscriber);
  if (it != end(subscribers_))
    subscribers_.erase(it);
}

void Dispatcher::notifyUserAdded(Subscriber *self,
                                 long long id,
                                 const Wt::WString &name,
                                 const Wt::WColor &color)
{
  std::scoped_lock<std::mutex> lock(subscriberMutex_);

  for (auto subscriber : subscribers_) {
    if (self == subscriber)
      continue;
    server_->post(subscriber->sessionId(), [subscriber, id, name, color]{
      subscriber->userAdded().emit(id, name, color);
    });
  }
}

void Dispatcher::notifyUserChangedColor(Subscriber *self,
                                        long long id,
                                        const Wt::WColor &color)
{
  std::scoped_lock<std::mutex> lock(subscriberMutex_);

  for (auto subscriber : subscribers_) {
    if (self == subscriber)
      continue;
    server_->post(subscriber->sessionId(), [subscriber, id, color]{
      subscriber->userChangedColor().emit(id, color);
    });
  }
}

void Dispatcher::notifyCellValueChanged(Subscriber *self,
                                        long long puzzleId,
                                        std::pair<int, int> cellRef)
{
  std::scoped_lock<std::mutex> lock(subscriberMutex_);

  for (auto subscriber : subscribers_) {
    if (self == subscriber)
      continue;
    server_->post(subscriber->sessionId(), [subscriber, puzzleId, cellRef]{
      subscriber->cellValueChanged().emit(puzzleId, cellRef);
    });
  }
}

void Dispatcher::notifyCursorMoved(Subscriber *self,
                                   long long puzzleId,
                                   long long user,
                                   std::pair<int, int> cellRef,
                                   Wt::Orientation direction)
{
  {
    std::scoped_lock<std::mutex> lock(positionMutex_);

    auto it = std::find_if(begin(userPositions_), end(userPositions_), [user](const UserCursor &cursor) {
      return user == cursor.userId;
    });

    if (puzzleId == -1 ||
         cellRef == std::make_pair(-1, -1)) {
      if (it != end(userPositions_)) {
        userPositions_.erase(it);
      } else {
        return;
      }
    } else if (it != end(userPositions_)) {
      it->puzzleId = puzzleId;
      it->cellRef = cellRef;
      it->direction = direction;
    } else {
      userPositions_.push_back({puzzleId, user, cellRef, direction});
    }
  }

  {
    std::scoped_lock<std::mutex> lock(subscriberMutex_);

    for (auto subscriber : subscribers_) {
      if (self == subscriber)
        continue;
      server_->post(subscriber->sessionId(), [subscriber, puzzleId, user, cellRef, direction]{
        subscriber->cursorMoved().emit(puzzleId, user, cellRef, direction);
      });
    }
  }
}

std::vector<UserCursor> Dispatcher::userPositions() const
{
  std::scoped_lock<std::mutex> lock(positionMutex_);

  return userPositions_;
}

Subscriber::Subscriber(const std::string &sessionId)
  : sessionId_(sessionId)
{ }

}
