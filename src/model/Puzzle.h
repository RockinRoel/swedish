#ifndef SWEDISH_PUZZLE_H_
#define SWEDISH_PUZZLE_H_

#include <Wt/Dbo/Types.h>

namespace swedish {

class Puzzle final : public Wt::Dbo::Dbo<Puzzle> {
public:
  std::string path;

  template<typename Action>
  void persist(Action &a)
  {
    Wt::Dbo::field(a, path, "path");
  }
};

}

DBO_EXTERN_TEMPLATES(swedish::Puzzle)

#endif // SWEDISH_PUZZLE_H_
