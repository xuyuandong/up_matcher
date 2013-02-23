// Author: cs.xuyd@gmail.com

#include "base/logging.h"
#include "base/string_util.h"
#include "up_matcher.h"
#include "up_element.h"

using namespace std;

namespace up2 {

static const char RegexChars[] = {
  '.', '?', '*', '+', '[', '(', '{', '$', '^', '|'
};
static const int RegexCharNum = sizeof(RegexChars) / sizeof(char);

UrlMatcher::UrlMatcher(int shortcut_size) 
  : shortcut_size_(shortcut_size) {
  
}

UrlMatcher::~UrlMatcher() {

}

bool UrlMatcher::LoadPatternFile(const string& filename) {
  file::SimpleLineReader reader(filename, true);
  vector<string> lines;
  reader.ReadLines(&lines);

  vector<string> items;
  string regex, payload, shortcut;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (lines[i][0] == '#')
      continue;

    items.clear();
    SplitStringUsingSubstr(lines[i], "[^o^]", &items);

    regex = items[0];
    VLOG(2) << "pattern: " << regex;

    payload.clear();
    if (items.size() > 1) 
      payload = items[1];
    
    if (items.size() > 2) {
      shortcut = items[2];
      AddPattern(regex, payload, shortcut);
    } else {
      AddPattern(regex, payload);
    }
  }

  return true;
}

bool UrlMatcher::AddPattern(const string& regex, const string& payload) {
  CheckerElem elem(regex, payload);

  string::size_type star_pos = regex.find(".*");
  size_t prefix_len = regex.size();
  if (star_pos != string::npos) {
    prefix_len = star_pos;
  }
  
  string prefix;
  for (size_t i = 0; i < prefix_len; ++i) {
    char c = regex[i];
    if (c == '\\') {
      bool hit_regex_char = false;
      char d = (i + 1 < regex.size()) ? regex[i + 1] : '\0';
      for (int r = 0; d != '\0' && r < RegexCharNum; ++r) {
        if (d == RegexChars[r]) {
          hit_regex_char = true;
          break;
        }
      }
      if (hit_regex_char) {
        prefix.push_back(d);
        i += 1; // jump two chars
        continue;
      } else {
        break;  // stop, end of prefix
      }
    } else {
      bool hit_regex_char = false;
      for (int r = 0; r < RegexCharNum; ++r) {
        if (c == RegexChars[r]) {
          hit_regex_char = true;
          break;
        }
      }
      if (hit_regex_char) {
        break; // stop, end of prefix
      }
    }
    prefix.push_back(c);
  }

  if (!prefix.empty()) {
    int prefix_size = prefix.size();
    vector<CheckerElem>* ce_ptr = prefix_pattern_list_.ExactMatch(prefix, 0, prefix_size);
    if (ce_ptr != NULL) {
      ce_ptr->push_back(elem);
    } else {
      vector<CheckerElem> vec;
      vec.push_back(elem);
      prefix_pattern_list_.Insert(prefix, 0, prefix_size, vec, false);
    }
  } else {  // we use sequential search vector
    sequential_pattern_list_.push_back(elem);
  }

  return true;
}

bool UrlMatcher::AddPattern(const string& regex, const string& payload, const string& shortcut) {
  if (shortcut.size() != shortcut_size_) {
    LOG(ERROR) << "Shortcut has illegal length: " << shortcut;
    return false;
  }

  // create index
  int half_shortcut_size = shortcut_size_ / 2;
  for (int pos = 0; pos <= half_shortcut_size; ++pos) {
    int distance = half_shortcut_size - pos;
    JumperElem* je_ptr = index_map_.ExactMatch(shortcut, pos, half_shortcut_size);
    if (je_ptr != NULL) {
      if (distance < je_ptr->jumped_distance_)
        je_ptr->jumped_distance_ = distance;
    } else {
      JumperElem je(distance);
      index_map_.Insert(shortcut, pos, half_shortcut_size, je, false);
    }
  }
  // store data
  base::hash_map<string, vector<CheckerElem> >::iterator it;
  it = indexed_pattern_list_.find(shortcut);
  if (it != indexed_pattern_list_.end()) {
    it->second.push_back(CheckerElem(regex, payload));
  } else {
    vector<CheckerElem> vec;
    vec.push_back(CheckerElem(regex, payload));
    indexed_pattern_list_[shortcut] = vec;
  }
  
  return true;
}

bool UrlMatcher::Match(const string& url) const {
  string result;
  return Match(url, &result);
}

bool UrlMatcher::Match(const string& url, string* result) const {
  if (MatchIndexedPattern(url, result))
    return true;
  if (MatchPrefixedPattern(url, result))
    return true;
  return MatchSequentialPattern(url, result);
}

bool UrlMatcher::MatchIndexedPattern(const string& url, string* result) const {
  if (url.size() < shortcut_size_)
    return false;

  int end_pos = url.size() - 1;
  int cur_tail_pos = shortcut_size_ - 1;
  int half_shortcut_size = shortcut_size_ / 2;

  while (cur_tail_pos <= end_pos) {
    int search_pos = cur_tail_pos - half_shortcut_size + 1;
    JumperElem* je_ptr = index_map_.ExactMatch(url, search_pos, half_shortcut_size);
    if (je_ptr != NULL) {
      int distance = je_ptr->jumped_distance_;
      if (distance == 0) {
        int shortcut_startpos = cur_tail_pos - shortcut_size_ + 1;
        const string& shortcut = url.substr(shortcut_startpos, shortcut_size_);

        base::hash_map<string, vector<CheckerElem> >::const_iterator it;
        it = indexed_pattern_list_.find(shortcut);
        if (it != indexed_pattern_list_.end()) {
          vector<CheckerElem>::const_iterator iter = it->second.begin();
          for (; iter != it->second.end(); ++iter) {
            if (re2::RE2::FullMatch(url, *(iter->normalized_regx_))) {
              *result = iter->payload_;
              return true;
            }
          }
        }

        cur_tail_pos++;
      } else {
        cur_tail_pos += distance;
      } 
    } else {
      cur_tail_pos += half_shortcut_size + 1;
    }
  }

  return false;
}

bool UrlMatcher::MatchPrefixedPattern(const string& url, string* result) const {
  int url_size = url.size();
  vector<CheckerElem>* ce_ptr = prefix_pattern_list_.PrefixMatch(url, 0, url_size);
  if (ce_ptr != NULL) {
    vector<CheckerElem>::const_iterator it;
    for (it = ce_ptr->begin(); it != ce_ptr->end(); ++it) {
      const CheckerElem& ce = *it;
      if (re2::RE2::FullMatch(url, *(ce.normalized_regx_))) {
        *result = ce.payload_;
        return true;
      }
    }
  }
  return false;
}

bool UrlMatcher::MatchSequentialPattern(const string& url, string* result) const {
  for (size_t i = 0; i < sequential_pattern_list_.size(); ++i) {
    const CheckerElem& ce = sequential_pattern_list_[i];
    if (re2::RE2::FullMatch(url, *(ce.normalized_regx_))) {
      *result = ce.payload_;
      return true;
    }
  }
  return false;
}

bool UrlMatcher::MatchAll(const string& url, vector<string>* result) const {
  return false;
}

const string* UrlMatcher::MatchRe(const string& url) const {
  return NULL;
}

const string* UrlMatcher::MatchRe(const string& url, string* result) const {
  return NULL;
}

}  // end namespace
