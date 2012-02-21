#ifndef _SRC_HEAP_H_
#define _SRC_HEAP_H_

//
// Heap is split into two parts:
//
//  * new space - all objects will be allocated here
//  * old space - tenured objects will be placed here
//
// Both spaces are lists of allocated buffers(pages) with a stack structure
//

#include "utils.h"
#include <stdint.h> // uint32_t

namespace dotlang {

class Space {
 public:
  class Page {
   public:
    Page(uint32_t size) {
      data_ = new char[size];
      top_ = data_;
      limit_ = data_ + size;
    }
    ~Page() {
      delete[] data_;
    }

    char* data_;
    char* top_;
    char* limit_;
  };

  Space(uint32_t page_size);

  // Move to next page where are at least `bytes` free
  // Otherwise allocate new page
  char* Allocate(uint32_t bytes);

  // Both top and limit are always pointing to current page's
  // top and limit.
  inline char** top() { return top_; }
  inline char** limit() { return limit_; }

 private:
  char** top_;
  char** limit_;

  inline void select(Page* page);

  List<Page*, EmptyClass> pages_;
  uint32_t page_size_;
};

class Heap {
 public:
  enum HeapTag {
    kNone,
    kContext,
    kNumber
  };

  Heap(uint32_t page_size) : new_space_(page_size), old_space_(page_size) {
  }

  inline Space* new_space() { return &new_space_; }
  inline Space* old_space() { return &old_space_; }

 private:
  Space new_space_;
  Space old_space_;
};

} // namespace dotlang

#endif // _SRC_HEAP_H_