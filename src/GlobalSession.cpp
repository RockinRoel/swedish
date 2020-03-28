#include "GlobalSession.h"

#include <Wt/WLogger.h>

#include <Wt/Dbo/Transaction.h>

#include <algorithm>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

#define TIMER 0

namespace {

constexpr const std::chrono::seconds interval = 3s;

}

namespace swedish {

// TODO(Roel): cleanly terminate global sync???

GlobalSession::GlobalSession(Wt::WIOService *ioService,
                             std::unique_ptr<Wt::Dbo::SqlConnection> conn)
  : ioService_(ioService),
    timer_(*ioService_),
    session_(std::move(conn)),
    terminated_(false)
{
  try {
    session_.createTables();

    {
      Wt::Dbo::Transaction t(session_);

      auto puzzle = Wt::Dbo::make_ptr<Puzzle>();
      puzzle.modify()->path = "/puzzle.jpg";
      puzzle.modify()->rotation = Rotation::Clockwise90;
      puzzle.modify()->width = 3000;
      puzzle.modify()->height = 4000;
      auto &rows = puzzle.modify()->rows_;

      constexpr int maxChar = static_cast<int>(Character::IJ);
      int counter = 0;
      for (int r = 0; r < 10; ++r) {
        auto &row = rows.emplace_back();
        for (int c = 0; c < 10; ++c) {
          auto &cell = row.emplace_back();
          cell.square = Wt::WRectF(10 + c * 30, 10 + r * 30, 30, 30);
          cell.character_ = static_cast<Character>(counter % (maxChar + 1));
          ++counter;
        }
      }
      rows[4][4].square = Wt::WRectF();
      rows[5][5].square = Wt::WRectF();
      rows[3][2].square = Wt::WRectF();

      session_.add(puzzle);
    }
  } catch (Wt::Dbo::Exception &e) {
    Wt::log("info") << "swedish::GlobalSession" << ": Caught exception: " << e.what();
    Wt::log("info") << "swedish::GlobalSession" << ": Assuming tables already exist and continuing";
  }

  session_.setFlushMode(Wt::Dbo::FlushMode::Manual);

#if TIMER
  timer_.expires_after(interval);
  timer_.async_wait(std::bind(&GlobalSession::timeout, this, std::placeholders::_1));
#endif
}

GlobalSession::~GlobalSession()
{
  {
    std::scoped_lock<std::mutex> lock(mutex_);
    terminated_ = true;
  }

  Wt::log("info") << "GlobalSession" << ": in dtor, canceling timer";

  try {
    Wt::log("info") << "GlobalSession" << ": last sync";
    sync(true);
  } catch (...) {
    // TODO(Roel): error!
  }
}

std::pair<Character, long long> GlobalSession::charAt(long long puzzle,
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

void GlobalSession::updateChar(long long puzzle,
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
  if (terminated_ || errc)
    return;

  sync(false);

  if (!terminated_) {
#if TIMER
    timer_.expires_after(interval);
    timer_.async_wait(std::bind(&GlobalSession::timeout, this, std::placeholders::_1));
#endif
  }
}

void GlobalSession::sync(bool last)
{
  Wt::log("info") << "GlobalSession" << ": performing global sync";

  std::scoped_lock<std::mutex> lock(mutex_);

  if (terminated_ && !last)
    return;

  Wt::Dbo::Transaction t(session_);

  for (auto &puzzle : puzzles_) {
    puzzle.flush();
  }
}

}
