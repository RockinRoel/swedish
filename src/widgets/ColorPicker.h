#ifndef SWEDISH_COLORPICKER_H_
#define SWEDISH_COLORPICKER_H_

#include <Wt/WColor.h>
#include <Wt/WCompositeWidget.h>

namespace swedish {

class ColorPicker final : public Wt::WCompositeWidget {
public:
  ColorPicker(const Wt::WColor &color = Wt::WColor());

  virtual ~ColorPicker() override;

  const Wt::WColor &pickedColor() const { return pickedColor_; }

  Wt::Signal<const Wt::WColor &> &colorChanged() { return colorChanged_; }

private:
  Wt::WColor pickedColor_;
  Wt::Signal<const Wt::WColor &> colorChanged_;
  Wt::WSlider *hueSlider_, *saturationSlider_, *lightnessSlider_;
  Wt::WSpinBox *hueSpinbox_, *saturationSpinbox_, *lightnessSpinbox_;
  Wt::WContainerWidget *currentColor_;

  Wt::WTemplate *impl();
  void sliderChanged(const Wt::WSlider *slider, Wt::WSpinBox *spinBox);
  void spinboxChanged(Wt::WSlider *slider, const Wt::WSpinBox *spinBox);
  void valueChanged();
};

}

#endif // SWEDISH_COLORPICKER_H_
