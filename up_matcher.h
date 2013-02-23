// Author: cs.xuyd@gmail.com

#ifndef UP2_UP_MATCHER_H_
#define UP2_UP_MATCHER_H_

#include <vector>
#include <string>
#include "base/hash_tables.h"
#include "up_trietree.h"

namespace up2 {

class JumperElem;
class CheckerElem;

class UrlMatcher {
 public:
  explicit UrlMatcher(int shortcut_size = 8);
  ~UrlMatcher();

  bool LoadPatternFile(const std::string& filename);

  bool AddPattern(const std::string& regex, const std::string& payload);
  bool AddPattern(const std::string& regex, const std::string& payload, const std::string& shortcut);

  bool Match(const std::string& url) const;

  bool Match(const std::string& url, std::string* result) const;

  bool MatchAll(const std::string& url, std::vector<std::string>* result) const;

  const std::string* MatchRe(const std::string& url) const;
  
  const std::string* MatchRe(const std::string& url, std::string* result) const;

 private: 
  bool MatchIndexedPattern(const std::string& url, std::string* result) const;
  bool MatchPrefixedPattern(const std::string& url, std::string* result) const;
  bool MatchSequentialPattern(const std::string& url, std::string* result) const;

  // search the shortcut index
  int shortcut_size_;
  UpTrieTree<JumperElem> index_map_;
  base::hash_map<std::string, std::vector<CheckerElem> > indexed_pattern_list_;

  // prefix search index
  UpTrieTree<std::vector<CheckerElem> > prefix_pattern_list_;
  
  // sequential search one by one
  std::vector<CheckerElem> sequential_pattern_list_;
};

}

#endif  // UP2_UP_MATCHER_H_
