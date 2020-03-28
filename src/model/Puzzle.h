#ifndef SWEDISH_PUZZLE_H_
#define SWEDISH_PUZZLE_H_

#include "../Rotation.h"

#include <Wt/WRectF.h>

#include <Wt/Dbo/Types.h>

#include <Wt/Json/Object.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace swedish {

enum class Character : std::uint8_t {
  None = 0,
  A = 1,
  B = 2,
  C = 3,
  D = 4,
  E = 5,
  F = 6,
  G = 7,
  H = 8,
  I = 9,
  J = 10,
  K = 11,
  L = 12,
  M = 13,
  N = 14,
  O = 15,
  P = 16,
  Q = 17,
  R = 18,
  S = 19,
  T = 20,
  U = 21,
  V = 22,
  W = 23,
  X = 24,
  Y = 25,
  Z = 26,
  IJ = 27
};

extern std::string_view charToStr(Character character);

extern Character strToChar(std::string_view str);

struct Cell final {
  Wt::WRectF square;
  Character character_ = Character::None;
  long long user_ = -1;

  inline bool isNull() const noexcept
  {
    return square.isNull();
  }
};

class Puzzle final : public Wt::Dbo::Dbo<Puzzle> {
public:
  std::string path;
  Rotation rotation = Rotation::None;
  int width = 0;
  int height = 0;

  using Row = std::vector<Cell>;
  std::vector<Row> rows_;

  Cell &cell(int row, int col) { return rows_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]; }
  const Cell &cell(int row, int col) const { return rows_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]; }

  template<typename Action>
  void persist(Action &a)
  {
    Wt::Dbo::field(a, path, "path");

    Wt::Json::Object data;
    if (a.getsValue()) {
      data = encodeJson();
    }

    Wt::Dbo::field(a, data, "data");

    if (a.setsValue()) {
      decodeJson(data);
    }
  }

private:
  Wt::Json::Object encodeJson() const;
  void decodeJson(const Wt::Json::Object &json);
};

}

DBO_EXTERN_TEMPLATES(swedish::Puzzle)

#endif // SWEDISH_PUZZLE_H_
