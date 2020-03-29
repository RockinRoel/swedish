#include "SharedSession.h"

#include <Wt/WLogger.h>

#include <Wt/Dbo/Transaction.h>

#include <algorithm>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace {

constexpr const std::chrono::seconds interval = 3s;

}

namespace swedish {

SharedSession::SharedSession(Wt::WIOService *ioService,
                             std::unique_ptr<Wt::Dbo::SqlConnection> conn)
  : ioService_(ioService),
    session_(std::move(conn)),
    terminated_(false)
{
  try {
    session_.createTables();
  } catch (Wt::Dbo::Exception &e) {
    Wt::log("info") << "swedish::SharedSession" << ": Caught exception: " << e.what();
    Wt::log("info") << "swedish::SharedSession" << ": Assuming tables already exist and continuing";
  }

  session_.setFlushMode(Wt::Dbo::FlushMode::Manual);
}

SharedSession::~SharedSession()
{
  {
    std::scoped_lock<std::mutex> lock(mutex_);
    terminated_ = true;
  }

  try {
    Wt::log("info") << "SharedSession" << ": last sync";
    sync(true);
  } catch (const Wt::Dbo::Exception &e) {
    Wt::log("error") << "SharedSession" << ": an error occurred when syncing: " << e.what();
  }
}

void SharedSession::startTimer()
{
  timer_ = std::make_unique<boost::asio::steady_timer>(*ioService_);
  timer_->expires_after(interval);
  timer_->async_wait(std::bind(&SharedSession::timeout, shared_from_this(), std::placeholders::_1));
}

void SharedSession::stopTimer()
{
  timer_->cancel();
  timer_ = nullptr;
}

std::pair<Character, long long> SharedSession::charAt(long long puzzle,
                                                      std::pair<int, int> cellRef)
{
  if (terminated_)
    return { Character::None, -1 };

  std::scoped_lock<std::mutex> lock(mutex_);

  Wt::Dbo::ptr<Puzzle> puzzlePtr = getPuzzle(puzzle);

  if (!puzzlePtr)
    return { Character::None, -1 }; // TODO(Roel): error!

  const Cell &cell = puzzlePtr->rows_[static_cast<std::size_t>(cellRef.first)][static_cast<std::size_t>(cellRef.second)];

  return { cell.character_, cell.user_ };
}

void SharedSession::updateChar(long long puzzle,
                               std::pair<int, int> cellRef,
                               Character character,
                               long long user)
{
  if (terminated_)
    return;

  std::scoped_lock<std::mutex> lock(mutex_);

  Wt::Dbo::ptr<Puzzle> puzzlePtr = getPuzzle(puzzle);

  if (!puzzlePtr)
    return;

  Cell &cell = puzzlePtr.modify()->rows_[static_cast<std::size_t>(cellRef.first)][static_cast<std::size_t>(cellRef.second)];

  if (cell.character_ == character) {
    return;
  }

  cell.character_ = character;
  cell.user_ = user;
}

Wt::Dbo::ptr<Puzzle> SharedSession::getPuzzle(long long puzzle)
{
  auto it = std::find_if(begin(puzzles_), end(puzzles_), [puzzle](const Wt::Dbo::ptr<Puzzle> &puzzlePtr) {
    return puzzle == puzzlePtr.id();
  });

  if (it == end(puzzles_)) {
    Wt::Dbo::Transaction t(session_);

    Wt::Dbo::ptr<Puzzle> puzzlePtr = session_.load<Puzzle>(puzzle);

    if (puzzlePtr) {
      puzzles_.push_back(puzzlePtr);
    }

    return puzzlePtr;
  } else {
    return *it;
  }
}

void SharedSession::timeout(boost::system::error_code errc)
{
  if (errc || terminated_)
    return;

  sync(false);

  if (!terminated_) {
    timer_->expires_after(interval);
    timer_->async_wait(std::bind(&SharedSession::timeout, shared_from_this(), std::placeholders::_1));
  }
}

void SharedSession::sync(bool last)
{
  Wt::log("info") << "SharedSession" << ": performing global sync";

  std::scoped_lock<std::mutex> lock(mutex_);

  if (terminated_ && !last)
    return;

  Wt::Dbo::Transaction t(session_);

  session_.flush();
}

}
