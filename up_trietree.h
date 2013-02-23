/*
  Author: Xu Yuandong (cs.xuyd@gmail.com)
*/

#ifndef UP2_UP_TRIETREE_H_
#define UP2_UP_TRIETREE_H_

#include <string>

namespace up2 {

/*
   Adblock's essencial idea is the Boyer-Moore liked
   shifting methods, trie-tree is perfect for this feature
   So I switches to this simple trie-tree, and obtains very
   good performance
*/

/* 
 * =========================================================================
 * UpTrieNode Declaration: 
 *
*/
const int kMaxNodeNum = 256;

template <typename tp_val>
class UpTrieNode {
 public:
  UpTrieNode();
  ~UpTrieNode();
  // Place for holding the value
  tp_val* val_ptr_;
  // Next array
  UpTrieNode<tp_val>* next_[kMaxNodeNum];
 private:
};

/* 
 * =========================================================================
 * UpTrieTree Declaration: 
 *
*/
template <typename tp_val>
class UpTrieTree {
 public:
  UpTrieTree();
  // De-constructor will releases the memory taken by trie-tree
  ~UpTrieTree();

  // Clear all the contents of the trie-tree, release memory
  void Clear();
  // Insert a <str, node_val> pair into the trie-tree
  // Parameter:
  // str: the key target string
  // start_pos: start from which position of str to insert as key
  // len: substring start from start_pos with length len as key
  // node_val: the value to insert
  // overide: if the key and value exists, overide or not
  // Return value:
  // true, succeeded
  // false, failed
  bool Insert(const std::string& str, const int start_pos, const int len,
      const tp_val& node_val, const bool overide);
  // Check if any prefix of the target string matches any keys
  // Parameter:
  // str: the key target string
  // start_pos: start from which position of str to insert as key
  // len: substring start from start_pos with length len as key
  // Return value:
  // the value of the first matched key in the trie-tree
  tp_val* PrefixMatch(const std::string& str, const int start_pos,
      const int len) const;
  // Check if the target string exactly matches any keys
  // Parameter:
  // str: the key target string
  // start_pos: start from which position of str to insert as key
  // len: substring start from start_pos with length len as key
  // Return value:
  // the value of the matched key in the trie-tree
  tp_val* ExactMatch(const std::string& str, const int start_pos,
      const int len) const;

 private:
  UpTrieNode<tp_val>* root_;
};

/* 
 * =========================================================================
 * UpTrieNode Implementation: 
 *
*/
template <typename tp_val>
UpTrieNode<tp_val>::UpTrieNode() {
  for (int index = 0; index < kMaxNodeNum; ++index) {
    next_[index] = NULL;
  }
  val_ptr_ = NULL;
}

template <typename tp_val>
UpTrieNode<tp_val>::~UpTrieNode() {
  for (int dict_index = 0; dict_index < kMaxNodeNum; ++dict_index) {
    if (next_[dict_index] != NULL) {
      delete next_[dict_index];
      next_[dict_index] = NULL;
    }
  }
  delete val_ptr_;
  val_ptr_ = NULL;
}

/* 
 * =========================================================================
 * UpTrieTree Implementation: 
 *
*/
template <typename tp_val>
UpTrieTree<tp_val>::UpTrieTree() {
  root_ =NULL;
}

template <typename tp_val>
UpTrieTree<tp_val>::~UpTrieTree() {
  Clear();
}

template <typename tp_val>
void UpTrieTree<tp_val>::Clear() {
  if (root_ == NULL) {
    return;
  }
  delete root_;
  root_ = NULL;
}

template <typename tp_val>
bool UpTrieTree<tp_val>::Insert(const std::string& str,
    const int start_pos, const int len, const tp_val& node_val,
    const bool overide) {
  if (root_ == NULL) {
    root_ = new UpTrieNode<tp_val>();
  }
  int str_len = str.length();
  int end_pos = start_pos + len - 1;
  if (start_pos >= str_len || end_pos >= str_len) {
    return false;
  }
  UpTrieNode<tp_val>* cur_node = root_;
  for (int pos = start_pos; pos <= end_pos; pos++) {
    int c_val = str[pos];
    //if (c_val < 0  || c_val >= kMaxNodeNum) {
    if (c_val < -128  || c_val > 127) {
      return false;
    }
    if (c_val < 0)  // modified
      c_val = 127 - c_val;
    if (cur_node->next_[c_val] == NULL) {
      cur_node->next_[c_val] = new UpTrieNode<tp_val>();
    }
    cur_node = cur_node->next_[c_val];
  }
  if (cur_node->val_ptr_ != NULL) {
    if (!overide) {
      return true;
    }
    delete cur_node->val_ptr_;
    cur_node->val_ptr_ = NULL;
  }
  cur_node->val_ptr_ = new tp_val(node_val);
  return true;
}

template <typename tp_val>
tp_val* UpTrieTree<tp_val>::PrefixMatch(const std::string& str,
    const int start_pos, const int len) const {
  if (root_ == NULL) {
    return NULL;
  }
  int str_len = str.length();
  int end_pos = start_pos + len - 1;
  if (start_pos >= str_len || end_pos >= str_len) {
    return NULL;
  }
  UpTrieNode<tp_val>* cur_node = root_;
  tp_val* last_match_ptr = NULL;
  int str_index = start_pos;
  for (; str_index <= end_pos; str_index++) {
    int c_val = str[str_index];
    //if (c_val < 0  || c_val >= kMaxNodeNum) {
    if (c_val < -128  || c_val > 127) {
      return last_match_ptr;
    }
    if (c_val < 0)  // modified
      c_val = 127 - c_val;
    if (cur_node->next_[c_val] == NULL) {
      return last_match_ptr;
    }
    if (cur_node->next_[c_val]->val_ptr_ != NULL) {
      last_match_ptr = cur_node->next_[c_val]->val_ptr_;
    }
    cur_node = cur_node->next_[c_val];
  }
  if (cur_node->val_ptr_ != NULL) {
    last_match_ptr = cur_node->val_ptr_;
  }
  return last_match_ptr;
}

template <typename tp_val>
tp_val* UpTrieTree<tp_val>::ExactMatch(const std::string& str,
    const int start_pos, const int len) const {
  if (root_ == NULL) {
    return NULL;
  }
  int str_len = str.length();
  int end_pos = start_pos + len - 1;
  if (start_pos >= str_len || end_pos >= str_len) {
    return NULL;
  }
  UpTrieNode<tp_val>* cur_node = root_;
  int str_index = start_pos;
  for (; str_index <= end_pos; str_index++) {
    int c_val = str[str_index];
    //if (c_val < 0  || c_val >= kMaxNodeNum) {
    if (c_val < -128  || c_val > 127) {
      return NULL;
    }
    if (c_val < 0)  // modified
      c_val = 127 - c_val;
    if (cur_node->next_[c_val] == NULL) {
      return NULL;
    }
    cur_node = cur_node->next_[c_val];
  }
  if (cur_node->val_ptr_ != NULL) {
    return cur_node->val_ptr_;
  }
  return NULL;
}

}  // namespace up2

#endif  // UP2_UP_TRIETREE_H_
