#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLogger.h>
#include <Wt/WServer.h>

#include <Wt/Dbo/Logger.h>

#include <Wt/Dbo/backend/Postgres.h>

#include "model/Session.h"
#include "model/User.h"

#include "widgets/ColorPicker.h"

#include <memory>

void testCreateDbAndOneUser(int argc, char *argv[]) {
  Wt::Dbo::logToWt();

  Wt::WServer server(argv[0]);

  server.setServerConfiguration(argc, argv);

  auto conn = std::make_unique<Wt::Dbo::backend::Postgres>(
        "user=roel password=hypersecure port=5432 dbname=swedish host=127.0.0.1");

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
  auto theme = std::make_shared<Wt::WBootstrapTheme>();
  theme->setVersion(Wt::BootstrapVersion::v3);

  return Wt::WRun(argc, argv, [&theme](const Wt::WEnvironment &env) {
    auto app = std::make_unique<Wt::WApplication>(env);
    app->setTheme(theme);

    app->messageResourceBundle().use(app->appRoot() + "template");
    app->root()->addNew<swedish::ColorPicker>();

    return app;
  });
}
