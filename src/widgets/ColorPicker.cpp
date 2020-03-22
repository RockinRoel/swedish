#include "ColorPicker.h"

#include <Wt/WApplication.h>
#include <Wt/WStringStream.h>

#include <cstdlib>
#include <iostream>

namespace swedish {

ColorPicker::ColorPicker(Wt::WColor color)
{
  Wt::WApplication *app = Wt::WApplication::instance();
  app->require("/js/jscolor.js");

  addStyleClass("jscolor");

  if (color.isDefault())
    color = Wt::WColor(Wt::StandardColor::Red);

  Wt::WStringStream js;
  js << "jscolor.installByClassName('jscolor');"
     << jsRef() + ".jscolor.fromRGB("
     << color.red() << ','
     << color.green() << ','
     << color.blue() << ");";
  doJavaScript(js.str());
}

ColorPicker::~ColorPicker()
{ }

const Wt::WColor ColorPicker::pickedColor() const
{
  const std::string s = text().toUTF8();
  char *end = nullptr;
  long rgb = std::strtol(s.c_str(), &end, 16);

  if (end != &*s.end())
    return Wt::WColor();

  const int b = rgb & 0xFF;
  const int g = (rgb >> 8) & 0xFF;
  const int r = (rgb >> 16) & 0xFF;

  return Wt::WColor(r, g, b);
}

}
