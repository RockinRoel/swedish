#include "Session.h"

#include <Wt/WLogger.h>

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
  mapClass<User>("users");

  try {
    createTables();
  } catch (Wt::Dbo::Exception &e) {
    Wt::log("info") << "swedish::Session" << ": Caught exception: " << e.what();
    Wt::log("info") << "swedish::Session" << ": Assuming tables already exist and continuing";
  }
}

}
