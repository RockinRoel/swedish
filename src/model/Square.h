#ifndef SWEDISH_SQUARE_H_
#define SWEDISH_SQUARE_H_

#include <Wt/WRectF.h>

namespace swedish {

struct Square final {
  Wt::WRectF rect;
  int row = 0, col = 0;
};

}

#endif // SWEDISH_SQUARE_H_
