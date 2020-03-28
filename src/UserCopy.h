#ifndef SWEDISH_USERCOPY_H_
#define SWEDISH_USERCOPY_H_

#include <Wt/WColor.h>
#include <Wt/WString.h>

namespace swedish {

struct UserCopy {
  long long id;
  Wt::WString name;
  Wt::WColor color;
};

}

#endif // SWEDISH_USERCOPY_H_
