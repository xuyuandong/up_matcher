/*
  This interface providing the following functionalities:
  (1) read in normalized regular expressions, and indexing the data
  (2) read in a string and tell if it hit a pattern
  Author: Xu Yuandong (cs.xuyd@gmail.com)
*/

#ifndef UP2_UP_ELEMENT_H_
#define UP2_UP_ELEMENT_H_

#include <string>
#include "re2/re2.h"

namespace up2 {

// The jumper element for fast query
class JumperElem {
 public:
  JumperElem()
      : jumped_distance_(0) {
  }
  JumperElem(const JumperElem& je)
      : jumped_distance_(je.jumped_distance_) {
  }
  explicit JumperElem(const int jumped_distance)
      : jumped_distance_(jumped_distance) {
  }
 public:
  // The distance for jumping when mathched
  int jumped_distance_;
};

// The checking element for holding a spacific normalized filter rule
class CheckerElem {
 public:
  CheckerElem()
      : regx_str_(""), normalized_regx_(NULL), payload_("") {
  }
  CheckerElem(const CheckerElem& ce)
      : regx_str_(ce.regx_str_),
      normalized_regx_(new re2::RE2(ce.regx_str_)),
      payload_(ce.payload_) {
  }
  CheckerElem(const std::string& regx_str, const std::string& payload)
      : regx_str_(regx_str),
      normalized_regx_(new re2::RE2(regx_str)),
      payload_(payload) {
  }
  ~CheckerElem() {
    if (normalized_regx_) {
      delete normalized_regx_;
      normalized_regx_ = NULL;
    }
  }
 public:
  // The normalized regulation expression string
  std::string regx_str_;
  // The normalized regulation expression
  re2::RE2* normalized_regx_;
  // The payload string
  std::string payload_;
};



}

#endif  // UP2_UP_ELEMENT_H_
