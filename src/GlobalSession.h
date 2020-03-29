#ifndef SWEDISH_GLOBALSESSION_H_
#define SWEDISH_GLOBALSESSION_H_

#include "model/Puzzle.h"
#include "model/Session.h"

#include <Wt/WIOService.h>

#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace swedish {

class GlobalSession final : public std::enable_shared_from_this<GlobalSession> {
public:
  GlobalSession(Wt::WIOService *ioService,
                std::unique_ptr<Wt::Dbo::SqlConnection> conn);

  ~GlobalSession();

  void startTimer();
  void stopTimer();

  std::pair<Character, long long> charAt(long long puzzle,
                                         std::pair<int, int> cellRef);

  void updateChar(long long puzzle,
                  std::pair<int, int> cellRef,
                  Character character,
                  long long user);

private:
  Wt::WIOService *ioService_;
  std::unique_ptr<boost::asio::steady_timer> timer_;
  Session session_;
  std::mutex mutex_;
  std::vector<Wt::Dbo::ptr<Puzzle>> puzzles_;
  std::atomic_bool terminated_;

  // NOTE: NEED LOCK BEFORE CALLING THIS
  Wt::Dbo::ptr<Puzzle> getPuzzle(long long puzzle);
  void timeout(boost::system::error_code errc);
  void sync(bool last);
};

}

#endif // SWEDISH_GLOBALSESSION_H_
