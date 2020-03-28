#include "Session.h"

#include "Puzzle.h"
#include "User.h"

namespace swedish {

Session::Session(std::unique_ptr<Wt::Dbo::SqlConnection> conn)
{
  setConnection(std::move(conn));

  init();
}

Session::Session(Wt::Dbo::SqlConnectionPool &pool)
{
  setConnectionPool(pool);

  init();
}

Session::~Session()
{ }

void Session::init()
{
  mapClass<Puzzle>("puzzles");
  mapClass<User>("users");
}

}
