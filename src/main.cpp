#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLogger.h>
#include <Wt/WServer.h>

#include <Wt/Dbo/Logger.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

#include <Wt/Dbo/backend/Postgres.h>

#include "GlobalSession.h"

#include "model/Puzzle.h"
#include "model/Session.h"
#include "model/User.h"

#include "widgets/PuzzleView.h"

#include <memory>

void testCreateDbAndOneUser(int argc, char *argv[]) {
  Wt::Dbo::logToWt();

  Wt::WServer server(argv[0]);

  server.setServerConfiguration(argc, argv);

  auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(
        "user=roel password=hypersecure port=5432 dbname=swedish host=10.1.0.45");

  conn->setProperty("show-queries", "true");

  swedish::Session session(std::move(conn));

  {
    Wt::Dbo::Transaction t(session);
    auto user = session.addNew<swedish::User>();
    user.modify()->name = "Roel";
    user.modify()->color = Wt::WColor(Wt::StandardColor::Red);
  }

  {
    Wt::Dbo::Transaction t(session);
    auto user = session.find<swedish::User>().resultValue();

    Wt::log("info") << user->color.cssText();
  }
}

int main(int argc, char *argv[]) {
  using namespace swedish;

  Wt::Dbo::logToWt();

  Wt::WServer server(argv[0]);

  auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(
        "user=roel password=hypersecure port=5432 dbname=swedish host=10.1.0.45");

  GlobalSession globalSession(&server.ioService(),
                              conn->clone());

  conn->setProperty("show-queries", "true");

  Wt::Dbo::FixedSqlConnectionPool pool(std::move(conn), 10);

  server.setServerConfiguration(argc, argv);

  server.addEntryPoint(Wt::EntryPointType::Application,
                       [](const Wt::WEnvironment &env) {
          auto app = std::make_unique<Wt::WApplication>(env);
          auto theme = std::make_shared<Wt::WBootstrapTheme>();
          theme->setVersion(Wt::BootstrapVersion::v3);
          app->setTheme(theme);

          app->messageResourceBundle().use(app->appRoot() + "template");
          Wt::Dbo::ptr<Puzzle> puzzle = Wt::Dbo::make_ptr<Puzzle>();
          puzzle.modify()->path = "/puzzle.jpg";
          puzzle.modify()->rotation = Rotation::Clockwise90;
          puzzle.modify()->width = 3000;
          puzzle.modify()->height = 4000;
          auto puzzleView = app->root()->addNew<swedish::PuzzleView>(puzzle);
          puzzleView->resize(500, 500);

          return app;
  });

  server.run();
}
