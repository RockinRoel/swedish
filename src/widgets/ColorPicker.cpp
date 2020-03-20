#include "ColorPicker.h"

#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WLength.h>
#include <Wt/WSlider.h>
#include <Wt/WSpinBox.h>
#include <Wt/WTemplate.h>

#include <cmath>

namespace {

Wt::WColor randomColor()
{
  // TODO(Roel): random color
  return Wt::WColor(Wt::StandardColor::Red);
}

}

namespace swedish {

ColorPicker::ColorPicker(const Wt::WColor &color)
  : Wt::WCompositeWidget(std::make_unique<Wt::WTemplate>(tr("tpl.swedish.colorpicker"))),
    pickedColor_(color),
    hueSlider_(impl()->bindNew<Wt::WSlider>("hue_slider")),
    saturationSlider_(impl()->bindNew<Wt::WSlider>("saturation_slider")),
    lightnessSlider_(impl()->bindNew<Wt::WSlider>("lightness_slider")),
    hueSpinbox_(impl()->bindNew<Wt::WSpinBox>("hue_spinbox")),
    saturationSpinbox_(impl()->bindNew<Wt::WSpinBox>("saturation_spinbox")),
    lightnessSpinbox_(impl()->bindNew<Wt::WSpinBox>("lightness_spinbox")),
    currentColor_(impl()->bindNew<Wt::WContainerWidget>("current_color"))
{
  currentColor_->resize(15, 15);

  hueSlider_->setRange(0, 360);
  hueSpinbox_->setRange(0, 360);
  saturationSlider_->setRange(0, 100);
  saturationSpinbox_->setRange(0, 100);
  lightnessSlider_->setRange(0, 100);
  lightnessSpinbox_->setRange(0, 100);

  if (pickedColor_.isDefault()) {
    pickedColor_ = randomColor();
  }

  double hsl[3];
  pickedColor_.toHSL(hsl);
  const int h = static_cast<int>(std::round(hsl[0]));
  const int s = static_cast<int>(std::round(hsl[1] * 100.0));
  const int l = static_cast<int>(std::round(hsl[2] * 100.0));
  hueSlider_->setValue(h);
  hueSpinbox_->setValue(h);
  saturationSlider_->setValue(s);
  saturationSpinbox_->setValue(s);
  lightnessSlider_->setValue(l);
  lightnessSpinbox_->setValue(l);

  currentColor_->decorationStyle().setBackgroundColor(pickedColor_);

  hueSlider_->valueChanged().connect(this, std::bind(&ColorPicker::sliderChanged, this, hueSlider_, hueSpinbox_));
  saturationSlider_->valueChanged().connect(this, std::bind(&ColorPicker::sliderChanged, this, saturationSlider_, saturationSpinbox_));
  lightnessSlider_->valueChanged().connect(this, std::bind(&ColorPicker::sliderChanged, this, lightnessSlider_, lightnessSpinbox_));

  hueSpinbox_->changed().connect(this, std::bind(&ColorPicker::spinboxChanged, this, hueSlider_, hueSpinbox_));
  saturationSpinbox_->changed().connect(this, std::bind(&ColorPicker::spinboxChanged, this, saturationSlider_, saturationSpinbox_));
  lightnessSpinbox_->changed().connect(this, std::bind(&ColorPicker::spinboxChanged, this, lightnessSlider_, lightnessSpinbox_));
}

ColorPicker::~ColorPicker()
{ }

Wt::WTemplate *ColorPicker::impl()
{
  return static_cast<Wt::WTemplate*>(implementation());
}

void ColorPicker::sliderChanged(const Wt::WSlider *slider, Wt::WSpinBox *spinBox)
{
  spinBox->setValue(slider->value());

  valueChanged();
}
void ColorPicker::spinboxChanged(Wt::WSlider *slider, const Wt::WSpinBox *spinBox)
{
  slider->setValue(spinBox->value());

  valueChanged();
}

void ColorPicker::valueChanged()
{
  const double h = hueSlider_->value();
  const double s = saturationSlider_->value() / 100.0;
  const double l = lightnessSlider_->value() / 100.0;
  pickedColor_ = Wt::WColor::fromHSL(h, s, l, 255);

  currentColor_->decorationStyle().setBackgroundColor(pickedColor_);

  colorChanged_(pickedColor_);
}

}
