#ifndef SWEDISH_SESSION_H_
#define SWEDISH_SESSION_H_

#include <Wt/Dbo/Session.h>

#include <memory>

namespace swedish {

class Session final : public Wt::Dbo::Session {
public:
  Session(std::unique_ptr<Wt::Dbo::SqlConnection> conn);
  Session(Wt::Dbo::SqlConnectionPool &pool);
  virtual ~Session() override;

private:
  void init();
};

}

#endif // SWEDISH_SESSION_H_
