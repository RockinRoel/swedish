#ifndef SWEDISH_COLORPICKER_H_
#define SWEDISH_COLORPICKER_H_

#include <Wt/WColor.h>
#include <Wt/WLineEdit.h>

namespace swedish {

class ColorPicker final : public Wt::WLineEdit {
public:
  ColorPicker(Wt::WColor color = Wt::WColor());

  virtual ~ColorPicker() override;

  const Wt::WColor pickedColor() const;
};

}

#endif // SWEDISH_COLORPICKER_H_
