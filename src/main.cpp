#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLogger.h>
#include <Wt/WServer.h>

#include <Wt/Dbo/Logger.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

#include <Wt/Dbo/backend/Postgres.h>

#include "Application.h"
#include "Dispatcher.h"
#include "GlobalSession.h"

#include "model/Puzzle.h"
#include "model/Session.h"
#include "model/User.h"

#include "widgets/PuzzleView.h"

#include <memory>

int main(int argc, char *argv[]) {
  using namespace swedish;

  Wt::Dbo::logToWt();

  Wt::WServer server(argc, argv);

  auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(
        "user=swedish password=hypersecure port=5432 dbname=swedish host=127.0.0.1");

  GlobalSession globalSession(&server.ioService(),
                              conn->clone());
  Dispatcher dispatcher(&server);

  conn->setProperty("show-queries", "true");

  Wt::Dbo::FixedSqlConnectionPool pool(std::move(conn), 10);

  server.addEntryPoint(Wt::EntryPointType::Application,
                       [&pool,&globalSession,&dispatcher](const Wt::WEnvironment &env) {
    return std::make_unique<Application>(env, pool, &globalSession, &dispatcher);
  });

  server.run();
}
