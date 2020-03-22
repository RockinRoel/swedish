#include "GlobalSession.h"

#include <Wt/Dbo/Transaction.h>

#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

namespace {

constexpr const std::chrono::seconds interval = 3s;

}

namespace swedish {

GlobalSession::GlobalSession(Wt::WIOService *ioService,
                             std::unique_ptr<Wt::Dbo::SqlConnection> conn)
  : ioService_(ioService),
    timer_(*ioService_),
    session_(std::move(conn))
{
  session_.setFlushMode(Wt::Dbo::FlushMode::Manual);
  timer_.expires_after(interval);
  timer_.async_wait(std::bind(&GlobalSession::timeout, this, std::placeholders::_1));
}

GlobalSession::~GlobalSession()
{
  timer_.cancel();

  try {
    sync();
  } catch (...) {
    // TODO(Roel): error!
  }
}

std::pair<Character, long long> GlobalSession::charAt(long long puzzle,
                                                      std::pair<int, int> cellRef)
{
  std::scoped_lock<std::mutex> lock(mutex_);

  Wt::Dbo::ptr<Puzzle> puzzlePtr = getPuzzle(puzzle);

  if (!puzzlePtr)
    return { Character::None, -1 }; // TODO(Roel): error!

  const Cell &cell = puzzlePtr->rows_[static_cast<std::size_t>(cellRef.first)][static_cast<std::size_t>(cellRef.second)];

  return { cell.character_, cell.user_ };
}

void GlobalSession::updateChar(long long puzzle,
                               std::pair<int, int> cellRef,
                               Character character,
                               long long user)
{
  std::scoped_lock<std::mutex> lock(mutex_);

  Wt::Dbo::ptr<Puzzle> puzzlePtr = getPuzzle(puzzle);

  if (!puzzlePtr)
    return;

  Cell &cell = puzzlePtr.modify()->rows_[static_cast<std::size_t>(cellRef.first)][static_cast<std::size_t>(cellRef.second)];

  cell.character_ = character;
  cell.user_ = user;
}

Wt::Dbo::ptr<Puzzle> GlobalSession::getPuzzle(long long puzzle)
{
  auto it = std::find_if(begin(puzzles_), end(puzzles_), [puzzle](const Wt::Dbo::ptr<Puzzle> &puzzlePtr) {
    return puzzle == puzzlePtr.id();
  });

  if (it == end(puzzles_)) {
    Wt::Dbo::Transaction t(session_);

    Wt::Dbo::ptr<Puzzle> puzzlePtr = session_.load<Puzzle>(puzzle);

    return puzzlePtr;
  } else {
    return *it;
  }
}

void GlobalSession::timeout(boost::system::error_code errc)
{
  if (errc)
    return;

  sync();

  timer_.expires_after(interval);
  timer_.async_wait(std::bind(&GlobalSession::timeout, this, std::placeholders::_1));
}

void GlobalSession::sync()
{
  std::scoped_lock<std::mutex> lock(mutex_);

  Wt::Dbo::Transaction t(session_);

  for (auto &puzzle : puzzles_) {
    puzzle.flush();
  }
}

}
