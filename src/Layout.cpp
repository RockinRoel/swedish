#include "Layout.h"

#include <Wt/WTemplate.h>

#include <memory>

namespace swedish {

Layout::Layout()
  : WCompositeWidget(std::make_unique<Wt::WTemplate>(tr("tpl.swedish.layout")))
{ }

Layout::~Layout()
{ }

Wt::WTemplate *Layout::impl()
{
  return static_cast<Wt::WTemplate *>(implementation());
}

}
