#include "Puzzle.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <Wt/Json/Array.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Json/Value.h>

#include <string_view>

using namespace std::string_view_literals;

namespace {

const std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"sv;

}

namespace swedish {

std::string_view charToStr(Character character)
{
  switch (character) {
  case Character::None:
    return ""sv;
  case Character::IJ:
    return "ij"sv;
  default:
    return alphabet.substr(static_cast<std::uint8_t>(character) - 1, 1);
  }
}

Character strToChar(std::string_view str)
{
  if (str.empty())
    return Character::None;
  else if (str == "ij"sv)
    return Character::IJ;
  else if (str.size() == 1 &&
           str[0] >= 'A' &&
           str[0] <= 'Z')
    return static_cast<Character>(static_cast<std::uint8_t>(str[0] - 'A') + 1);
  else
    return Character::None; // TODO(Roel): this is an error!
}

Wt::Json::Object Puzzle::encodeJson() const
{
  Wt::Json::Object result;

  result["width"] = width;
  result["height"] = height;
  result["rotation"] = rotationToDegrees(rotation);

  Wt::Json::Array &json_rows = result["rows"] = Wt::Json::Value(Wt::Json::Type::Array);

  for (const Row &row : rows_) {
    json_rows.push_back(Wt::Json::Value(Wt::Json::Type::Array));
    Wt::Json::Array &json_row = json_rows.back();
    for (const Cell &cell : row) {
      if (cell.empty()) {
        json_row.push_back(Wt::Json::Value(Wt::Json::Type::Null));
      } else {
        json_row.push_back(Wt::Json::Value(Wt::Json::Type::Object));
        Wt::Json::Object &json_cell = json_row.back();
        Wt::Json::Array &json_rect = json_cell["rect"] = Wt::Json::Value(Wt::Json::Type::Array);
        json_rect.push_back(static_cast<int>(cell.square.x()));
        json_rect.push_back(static_cast<int>(cell.square.y()));
        json_rect.push_back(static_cast<int>(cell.square.width()));
        json_rect.push_back(static_cast<int>(cell.square.height()));
        json_cell["value"] = Wt::utf8(std::string(charToStr(cell.character_)));
      }
    }
  }

  return result;
}

void Puzzle::decodeJson(const Wt::Json::Object &json)
{
  width = json.get("width");
  height = json.get("height");
  rotation = degreesToRotation(json.get("rotation"));

  rows_.clear();

  const Wt::Json::Array &json_rows = json.get("rows");

  for (const Wt::Json::Array &json_row : json_rows) {
    std::vector<Cell> &row = rows_.emplace_back();
    for (const Wt::Json::Value &json_cell : json_row) {
      if (json_cell.isNull()) {
        row.push_back(Cell());
      } else {
        const Wt::Json::Object &json_cell_o = json_cell;
        const Wt::Json::Array &json_rect = json_cell_o.get("rect");
        assert(json_rect.size() == 4);
        Cell &cell = row.emplace_back();
        cell.square = Wt::WRectF(json_rect[0].toNumber(),
                                 json_rect[1].toNumber(),
                                 json_rect[2].toNumber(),
                                 json_rect[3].toNumber());
        Wt::WString value = json_cell_o.get("value");
        cell.character_ = strToChar(value.toUTF8());
      }
    }
  }
}

}

DBO_INSTANTIATE_TEMPLATES(swedish::Puzzle)
