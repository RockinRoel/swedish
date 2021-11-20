// SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SWEDISH_USER_H_
#define SWEDISH_USER_H_

#include <Wt/WColor.h>

#include <Wt/Dbo/Types.h>

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>

#include <cstdlib>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

namespace Wt {
namespace Dbo {

template<>
struct sql_value_traits< ::Wt::WColor>
{
  static std::string type(SqlConnection *conn, int size)
  {
    return sql_value_traits<std::string>::type(conn, size);
  }

  static void bind(WColor c, SqlStatement *statement, int column, int size)
  {
    (void)size;

    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::hex
       << std::uppercase
       << std::setw(2)
       << std::setfill('0')
       << c.red();
    ss << std::hex
       << std::uppercase
       << std::setw(2)
       << std::setfill('0')
       << c.green();
    ss << std::hex
       << std::uppercase
       << std::setw(2)
       << std::setfill('0')
       << c.blue();

    std::string s = ss.str();
    statement->bind(column, s);
  }

  static bool read(WColor &c, SqlStatement *statement, int column, int size)
  {
    std::string s;
    s.reserve(6);
    bool result = statement->getResult(column, &s, size);
    if (!result)
      return false;
    assert(s.size() == 6); // TODO(Roel): not robust?
    char *end = nullptr;
    long rgb = std::strtol(s.c_str(), &end, 16);
    if (end != &*s.end())
      return false;

    const int b = rgb & 0xFF;
    const int g = (rgb >> 8) & 0xFF;
    const int r = (rgb >> 16) & 0xFF;

    c = Wt::WColor(r, g, b);
    return true;
  }
};

}
}

namespace swedish {

class User final : public Wt::Dbo::Dbo<User> {
public:
  Wt::WString name;
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
