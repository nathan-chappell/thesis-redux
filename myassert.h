// myassert.h

#include <cassert>
#include <iomanip>
#include <iostream>

#define MYASSERT(assertion)                                                    \
  {                                                                            \
    std::cerr << __FILE__ << ":" << __LINE__ << '\t' << "(" << #assertion      \
              << ") = " << std::boolalpha << (bool)(assertion) << std::endl;   \
    assert((assertion));                                                       \
  }

#define MYTEST(assertion)                                                      \
  {                                                                            \
    std::cerr << __FILE__ << ":" << __LINE__ << '\t' << "(" << #assertion      \
              << ") = " << std::boolalpha << (bool)(assertion) << std::endl;   \
  }

#define DIAGNOSTIC std::cerr << __FILE__ << ":" << __LINE__ << " "
