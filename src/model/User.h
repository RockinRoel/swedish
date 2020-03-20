#ifndef SWEDISH_USER_H_
#define SWEDISH_USER_H_

#include <Wt/WColor.h>

#include <Wt/Dbo/Types.h>

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>

#include <string>
#include <vector>

namespace Wt {
namespace Dbo {

template<>
struct sql_value_traits< ::Wt::WColor>
{
  static std::string type(SqlConnection *conn, int size)
  {
    return sql_value_traits<std::vector<unsigned char>>::type(conn, size);
  }

  static void bind(WColor c, SqlStatement *statement, int column, int size)
  {
    std::vector<unsigned char> data = {
      static_cast<unsigned char>(c.red()),
      static_cast<unsigned char>(c.green()),
      static_cast<unsigned char>(c.blue())
    };
    statement->bind(column, data);
  }

  static bool read(WColor &c, SqlStatement *statement, int column, int size)
  {
    std::vector<unsigned char> data;
    data.reserve(3);
    bool result = statement->getResult(column, &data, size);
    if (!result)
      return false;
    assert(data.size() == 3); // TODO(Roel): not robust?
    c = Wt::WColor(
          static_cast<int>(data[0]),
          static_cast<int>(data[1]),
          static_cast<int>(data[2]));
    return true;
  }
};

}
}

namespace swedish {

class User {
public:
  std::string name;
  Wt::WColor color;

  template<typename Action>
  void persist(Action &a)
  {
    Wt::Dbo::field(a, name, "name");
    Wt::Dbo::field(a, color, "color");
  }
};

}

DBO_EXTERN_TEMPLATES(swedish::User)

#endif // SWEDISH_USER_H_
